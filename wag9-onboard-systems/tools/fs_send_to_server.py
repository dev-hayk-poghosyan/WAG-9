"""Передатчик в станцию неисправностей WAG-9: манометры, лампы E-70, пантографы.

Заменяет симулятор: раз в 100 мс шлёт TrainSimToFaultStationMessage
с FaultStationIncomingMessage внутри. Давление в кгс/см², шкала 0..10.
Лампа: on или off, цвет у каждой свой и задан в окне станции.
Пантограф: up или down.

    python tools/fs_send_to_server.py
    python tools/fs_send_to_server.py --ccb 5 --e70 3.5 --psu-normal on
    python tools/fs_send_to_server.py --pantograph-1 up
    python tools/fs_send_to_server.py --sweep

Без аргументов открывается окно. --sweep качает обе стрелки, зажигает
лампы по очереди и поднимает-опускает пантографы. Окно wag9_fs должно быть уже запущено. Приёмник
fs_receive_from_server.py можно держать открытым одновременно: станция
отвечает первому клиенту, а пакеты принимает от всех.
"""

import argparse
import math
import struct
import sys
import threading
import time
import tkinter as tk
from tkinter import ttk

from fs_receive_from_server import ADDRESS, PeerTimeout, ZmtpClient

PERIOD_S = 0.1
MAX_PRESSURE = 10.0
SWEEP_PERIOD_S = 10.0
LAMP_PERIOD_S = 1.0
PANTOGRAPH_PERIOD_S = 3.0

# Номер поля в FaultStationIncomingMessage, имя ключа, подпись.
GAUGES = (
    (1, "ccb", "CCB"),
    (2, "e70", "E-70"),
)
LEDS = (
    (3, "processor_fault", "PROCESSOR FAULT"),
    (4, "system_fault", "SYSTEM FAULT"),
    (5, "psu_normal", "PSU NORMAL"),
)
PANTOGRAPHS = (
    (6, "pantograph_1", "Пантограф 1"),
    (7, "pantograph_2", "Пантограф 2"),
)
# SB1 relays and contactors: field number, key, caption, checkbox text, default.
# A tripped relay shows a red plate; an open contactor pops its button out.
SB1_STATES = (
    (8, "relay_control_electronics_off_tripped", "SB1 RELAY CONTROL ELECTRONICS OFF", "сработало", False),
    (9, "relay_time_delay_vcb_tripped", "SB1 RELAY TIME DELAY VCB", "сработало", False),
    (10, "power_supply_cab_contactor_closed", "SB1 POWER SUPPLY CAB CONTACTOR", "замкнут", True),
    (11, "auxiliary_contactor_vcb_closed", "SB1 AUXILIARY CONTACTOR VCB", "замкнут", True),
)
LAMP_STATES = ("off", "on")
POSITIONS = ("down", "up")


def encode_varint(value):
    out = bytearray()
    while True:
        byte = value & 0x7F
        value >>= 7
        if value:
            out.append(byte | 0x80)
        else:
            out.append(byte)
            return bytes(out)


def encode_bytes(number, payload):
    return encode_varint(number << 3 | 2) + encode_varint(len(payload)) + payload


def encode_double(number, value):
    return encode_varint(number << 3 | 1) + struct.pack("<d", value)


def encode_bool(number, value):
    return encode_varint(number << 3) + encode_varint(int(value))


def build_packet(state):
    """state: ключ из GAUGES/LEDS/PANTOGRAPHS -> значение. None или нет ключа — поле не шлём."""
    incoming = b""
    for number, key, _ in GAUGES:
        if state.get(key) is not None:
            incoming += encode_double(number, state[key])
    for number, key, _ in LEDS + PANTOGRAPHS:
        if state.get(key) is not None:
            incoming += encode_bool(number, state[key])
    for number, key, _, _, _ in SB1_STATES:
        if state.get(key) is not None:
            incoming += encode_bool(number, state[key])
    return encode_bytes(1, b"WAG9") + encode_bytes(2, incoming)


def sweep_pressure(started):
    phase = (time.monotonic() - started) / SWEEP_PERIOD_S * 2 * math.pi
    return MAX_PRESSURE / 2 * (1 - math.cos(phase))


def cycle_lamp(started, offset):
    """Лампы зажигаются по очереди, по одной на LAMP_PERIOD_S."""
    step = int((time.monotonic() - started) / LAMP_PERIOD_S)
    return step % len(LEDS) == offset


def cycle_pantograph(started, offset):
    """Пантографы поднимаются и опускаются по очереди, каждые PANTOGRAPH_PERIOD_S."""
    step = int((time.monotonic() - started) / PANTOGRAPH_PERIOD_S)
    return (step + offset) % 2 == 1


def run_sender(address, values, stop, on_status):
    """values() -> словарь для build_packet на каждом такте."""
    client = ZmtpClient()
    try:
        client.connect(address, timeout=0.02)
    except (ConnectionError, OSError, PeerTimeout) as error:
        raise RuntimeError(f"окно wag9_fs не слушает {address}") from error

    on_status(f"шлю в {address}")
    try:
        next_at = time.monotonic()
        while not stop.is_set():
            now = time.monotonic()
            if now >= next_at:
                client.send(build_packet(values()))
                next_at = now + PERIOD_S
            # Исходящие пакеты станции не нужны, но их надо вычитывать:
            # иначе не дойдут PING и забьётся буфер.
            try:
                client.recv()
            except PeerTimeout:
                pass
    except (ConnectionError, OSError) as error:
        raise RuntimeError(f"связь со станцией потеряна: {error}") from error
    finally:
        client.close()


class SenderWindow:
    def __init__(self, address):
        self._address = address
        self._stop = threading.Event()
        self._started = time.monotonic()
        self._lock = threading.Lock()
        self._current = {}

        self._root = tk.Tk()
        self._root.title("WAG-9 FS: в станцию")
        self._root.resizable(False, False)
        self._status = tk.StringVar(value=f"подключаюсь к {address}")
        ttk.Label(self._root, textvariable=self._status, padding=(8, 6)).grid(
            row=0, column=0, columnspan=4, sticky=tk.W)

        row = 1
        self._gauges = {}
        for _, key, title in GAUGES:
            enabled = tk.BooleanVar(value=True)
            value = tk.DoubleVar(value=0.0)
            text = tk.StringVar(value="0.0")
            ttk.Checkbutton(self._root, text=title, variable=enabled).grid(
                row=row, column=0, padx=8, sticky=tk.W)
            ttk.Scale(self._root, from_=0, to=MAX_PRESSURE, variable=value, length=320,
                      command=lambda _, v=value, t=text: t.set(f"{v.get():.1f}")).grid(
                row=row, column=1, padx=4, pady=4)
            ttk.Label(self._root, textvariable=text, width=5, anchor=tk.E).grid(row=row, column=2)
            ttk.Label(self._root, text="кгс/см²").grid(row=row, column=3, padx=(2, 8))
            self._gauges[key] = (enabled, value, text)
            row += 1

        ttk.Separator(self._root).grid(row=row, column=0, columnspan=4, sticky=tk.EW, pady=4)
        row += 1

        self._leds = {}
        for _, key, title in LEDS:
            enabled = tk.BooleanVar(value=True)
            lit = tk.BooleanVar(value=False)
            ttk.Checkbutton(self._root, text=title, variable=enabled).grid(
                row=row, column=0, padx=8, sticky=tk.W)
            ttk.Checkbutton(self._root, text="горит", variable=lit).grid(
                row=row, column=1, padx=4, pady=2, sticky=tk.W)
            self._leds[key] = (enabled, lit)
            row += 1

        ttk.Separator(self._root).grid(row=row, column=0, columnspan=4, sticky=tk.EW, pady=4)
        row += 1

        self._pantographs = {}
        for _, key, title in PANTOGRAPHS:
            enabled = tk.BooleanVar(value=True)
            raised = tk.BooleanVar(value=False)
            ttk.Checkbutton(self._root, text=title, variable=enabled).grid(
                row=row, column=0, padx=8, sticky=tk.W)
            ttk.Checkbutton(self._root, text="поднят", variable=raised).grid(
                row=row, column=1, padx=4, pady=2, sticky=tk.W)
            self._pantographs[key] = (enabled, raised)
            row += 1

        ttk.Separator(self._root).grid(row=row, column=0, columnspan=4, sticky=tk.EW, pady=4)
        row += 1

        self._sb1 = {}
        for _, key, title, text, default in SB1_STATES:
            enabled = tk.BooleanVar(value=True)
            value = tk.BooleanVar(value=default)
            ttk.Checkbutton(self._root, text=title, variable=enabled).grid(
                row=row, column=0, padx=8, sticky=tk.W)
            ttk.Checkbutton(self._root, text=text, variable=value).grid(
                row=row, column=1, padx=4, pady=2, sticky=tk.W)
            self._sb1[key] = (enabled, value)
            row += 1

        self._sweep = tk.BooleanVar(value=False)
        ttk.Checkbutton(self._root,
                        text=f"качать стрелки 0..{MAX_PRESSURE:g} за {SWEEP_PERIOD_S:g} с",
                        variable=self._sweep).grid(row=row, column=0, columnspan=4, padx=8,
                                                   pady=(6, 0), sticky=tk.W)
        row += 1
        self._cycle = tk.BooleanVar(value=False)
        ttk.Checkbutton(self._root, text=f"зажигать лампы по очереди раз в {LAMP_PERIOD_S:g} с",
                        variable=self._cycle).grid(row=row, column=0, columnspan=4, padx=8,
                                                   sticky=tk.W)
        row += 1
        self._toggle = tk.BooleanVar(value=False)
        ttk.Checkbutton(self._root,
                        text=f"поднимать и опускать пантографы раз в {PANTOGRAPH_PERIOD_S:g} с",
                        variable=self._toggle).grid(row=row, column=0, columnspan=4, padx=8,
                                                    pady=(0, 8), sticky=tk.W)

        self._root.protocol("WM_DELETE_WINDOW", self._close)
        threading.Thread(target=self._send, name="fs-sender", daemon=True).start()
        self._root.after(int(PERIOD_S * 1000), self._tick)

    def _tick(self):
        state = {}
        pressure = sweep_pressure(self._started) if self._sweep.get() else None
        for key, (enabled, value, text) in self._gauges.items():
            if pressure is not None:
                value.set(pressure)
                text.set(f"{pressure:.1f}")
            state[key] = value.get() if enabled.get() else None

        for offset, (key, (enabled, lit)) in enumerate(self._leds.items()):
            if self._cycle.get():
                lit.set(cycle_lamp(self._started, offset))
            state[key] = lit.get() if enabled.get() else None

        for offset, (key, (enabled, raised)) in enumerate(self._pantographs.items()):
            if self._toggle.get():
                raised.set(cycle_pantograph(self._started, offset))
            state[key] = raised.get() if enabled.get() else None

        for key, (enabled, value) in self._sb1.items():
            state[key] = value.get() if enabled.get() else None

        with self._lock:
            self._current = state
        if not self._stop.is_set():
            self._root.after(int(PERIOD_S * 1000), self._tick)

    def _values(self):
        with self._lock:
            return dict(self._current)

    def _send(self):
        try:
            run_sender(self._address, self._values, self._stop, self._set_status)
        except RuntimeError as error:
            self._set_status(str(error))

    def _set_status(self, text):
        self._root.after(0, self._status.set, text)

    def _close(self):
        self._stop.set()
        self._root.destroy()

    def run(self):
        self._root.mainloop()


def main():
    parser = argparse.ArgumentParser(description="Передатчик манометров, ламп и пантографов в WAG-9 FS")
    parser.add_argument("--addr", default=ADDRESS)
    parser.add_argument("--ccb", type=float, help="давление CCB, кгс/см²")
    parser.add_argument("--e70", type=float, help="давление E-70, кгс/см²")
    for _, key, title in LEDS:
        parser.add_argument("--" + key.replace("_", "-"), choices=LAMP_STATES, help=f"лампа {title}")
    for _, key, title in PANTOGRAPHS:
        parser.add_argument("--" + key.replace("_", "-"), choices=POSITIONS, help=title)
    parser.add_argument("--sweep", action="store_true",
                        help="без окна: качать стрелки, зажигать лампы по очереди, двигать пантографы")
    args = parser.parse_args()

    fixed = {"ccb": args.ccb, "e70": args.e70}
    for _, key, _ in LEDS:
        lamp = getattr(args, key)
        fixed[key] = None if lamp is None else lamp == "on"
    for _, key, _ in PANTOGRAPHS:
        position = getattr(args, key)
        fixed[key] = None if position is None else position == "up"

    if all(value is None for value in fixed.values()) and not args.sweep:
        SenderWindow(args.addr).run()
        return 0

    started = time.monotonic()

    def values():
        if not args.sweep:
            return fixed
        pressure = sweep_pressure(started)
        state = {"ccb": pressure, "e70": pressure}
        for offset, (_, key, _) in enumerate(LEDS):
            state[key] = cycle_lamp(started, offset)
        for offset, (_, key, _) in enumerate(PANTOGRAPHS):
            state[key] = cycle_pantograph(started, offset)
        return state

    stop = threading.Event()
    try:
        run_sender(args.addr, values, stop, lambda text: print(text, flush=True))
    except RuntimeError as error:
        print(error)
        return 1
    except KeyboardInterrupt:
        stop.set()
    return 0


if __name__ == "__main__":
    sys.exit(main())
