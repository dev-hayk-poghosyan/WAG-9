"""Приёмник станции неисправностей WAG-9.

Окно показывает положение каждого органа из исходящего пакета.
Сервер — ZMQ_SERVER. Скрипт сам говорит на ZMTP как ZMQ_CLIENT:
установленный pyzmq собран без этих сокетов и падает на рукопожатии.

    python tools/fs_receive_from_server.py
    python tools/fs_receive_from_server.py --check

Окно wag9_fs должно быть уже запущено. Адрес берётся из data/Wag9.xml.
"""

import argparse
import queue
import socket
import sys
import threading
import time
import tkinter as tk
from tkinter import ttk

ADDRESS = "tcp://127.0.0.1:12282"
KEEPALIVE_S = 3

# FaultStationInitMessage: vehicle_numbers = "WAG9"
INIT = b"\x0a\x04WAG9"

MESSAGES = {
    "PneumoCCB": {
        1: "ec_coc", 2: "tc2_coc", 3: "tc1_coc", 4: "sifa_coc", 5: "fp_coc",
        6: "sand1_coc", 7: "sand2_coc", 8: "ul_coc", 9: "cb_coc", 10: "pans_coc",
        11: "pan1_coc", 12: "pan2_coc", 13: "pan_sel_sw", 14: "pass_goods",
        15: "per_coc", 16: "dead_engine", 17: "gp",
    },
    "PneumoE70": {
        4: "pan_sel_sw", 5: "emergency_cock", 6: "brake_pipe_cock", 7: "pan1", 8: "pan2", 9: "vcb", 10: "pantog",
        11: "brake_feed_pipe", 12: "dead_engine", 13: "sand_1", 14: "sand_2",
    },
    "Sb1": {
        1: "sb1_failure_mode_operation", 2: "sb1_bogie_cut_out",
        3: "sb1_configuration_switch", 4: "sb1_vigilance_device_cut_off",
        5: "sb1_simulation", 6: "sb1_configuration_button", 7: "sb1_drivers_cab",
        8: "sb1_panto_vcb_control", 9: "sb1_24v_48v_power_supply", 10: "sb1_lighting_front",
        11: "sb1_electronics_traction_converter_1", 12: "sb1_power_supply_gate_units",
        13: "sb1_monitoring", 14: "sb1_electronics_aux_converter_1",
        15: "sb1_central_electronics_1", 16: "sb1_central_electronics_2", 17: "sb1_speed_sensor",
    },
    "Hb1": {
        1: "hb1_transformer_oil_pump", 2: "hb1_converter_oil_pump", 3: "hb1_main_compressor",
        4: "hb1_traction_motor_blower", 5: "hb1_scavenge_blower_tm", 6: "hb1_vent_oil_cooler",
        7: "hb1_mrb1", 8: "hb1_scavenge_blower_mr", 9: "hb1_cab_ventilation",
        10: "hb1_cab_heater", 11: "hb1_crew_fan",
    },
    "Sb2": {
        1: "sb2_auxiliary_compressor", 2: "sb2_drivers_cab", 3: "sb2_lighting_machine_room",
        4: "sb2_power_supply_gate_units", 5: "sb2_commissioning_1", 6: "sb2_vigilance_control",
        7: "sb2_pneumatic_panel", 8: "sb2_commissioning_2", 9: "sb2_24v_48v_power_supply",
        10: "sb2_marker_lights", 11: "sb2_lighting_front", 12: "sb2_electronics_traction_converter_2",
        13: "sb2_monitoring", 14: "sb2_electronics_aux_converter_2",
        15: "sb2_electronics_aux_converter_3", 16: "sb2_central_electronics_2",
        17: "sb2_central_electronics_3", 18: "sb2_memotel_speedometer", 19: "sb2_aux_contactor",
        20: "sb2_power_supply_air_dryer", 21: "sb2_mcb_for_air_dryer",
        22: "sb2_output_battery_charger", 23: "sb2_control_circuit_locomotive",
    },
    "Hb2": {
        1: "hb2_transformer_oil_pump", 2: "hb2_converter_oil_pump", 3: "hb2_main_compressor",
        4: "hb2_traction_motor_blower", 5: "hb2_scavenge_blower_tm", 6: "hb2_vent_oil_cooler",
        7: "hb2_m_c_room_blower", 8: "hb2_scavenge_blower_mr", 9: "hb2_circuit_breaker_for_cab_ac",
    },
    # Поля 1 и 2 (пантографы) зарезервированы: пантографы теперь приходят из симулятора.
    "FaultStationOutgoingMessage": {
        3: ("pneumo_ccb", "PneumoCCB"), 4: ("pneumo_e70", "PneumoE70"),
        5: ("sb1", "Sb1"), 6: ("hb1", "Hb1"), 7: ("sb2", "Sb2"), 8: ("hb2", "Hb2"),
    },
}

# Поля, которые у нормального поезда не равны нулю.
STARTUP = {
    "PneumoCCB": {"pans_coc": 1, "pan1_coc": 1, "pan2_coc": 1},
    "PneumoE70": {"emergency_cock": 1, "brake_pipe_cock": 1, "pan1": 1, "pan2": 1, "vcb": 1,
                  "pantog": 1, "brake_feed_pipe": 1, "sand_1": 1, "sand_2": 1},
}


def read_varint(buf, index):
    value = 0
    shift = 0
    while index < len(buf):
        byte = buf[index]
        index += 1
        value |= (byte & 0x7F) << shift
        if byte < 0x80:
            return value, index
        shift += 7
    raise ValueError("оборванный varint")


def parse_fields(buf):
    index = 0
    fields = []
    while index < len(buf):
        key, index = read_varint(buf, index)
        number, wire = key >> 3, key & 7
        if wire == 0:
            value, index = read_varint(buf, index)
        elif wire == 2:
            length, index = read_varint(buf, index)
            value = buf[index:index + length]
            index += length
        else:
            raise ValueError(f"поле {number}: провод {wire} здесь не нужен")
        fields.append((number, value))
    return fields


def decode_message(buf, schema_name):
    schema = MESSAGES[schema_name]
    decoded = {}
    for number, value in parse_fields(buf):
        spec = schema.get(number)
        if spec is None:
            decoded[f"#{number}"] = value
        elif isinstance(spec, tuple):
            decoded[spec[0]] = decode_message(value, spec[1])
        else:
            decoded[spec] = value
    return decoded


def unwrap(frame):
    vehicle = ""
    payload = b""
    for number, value in parse_fields(frame):
        if number == 1 and isinstance(value, (bytes, bytearray)):
            vehicle = value.decode("utf-8", "replace")
        elif number == 2 and isinstance(value, (bytes, bytearray)):
            payload = value
    state = decode_message(payload, "FaultStationOutgoingMessage")
    return vehicle, state


def flatten(state, prefix=""):
    flat = {}
    for key, value in state.items():
        path = f"{prefix}{key}"
        if isinstance(value, dict):
            flat[path] = "{}"
            flat.update(flatten(value, path + "."))
        else:
            flat[path] = value
    return flat


def startup_errors(state):
    errors = []
    blocks = [name for name in ("pneumo_ccb", "pneumo_e70") if name in state]
    if len(blocks) != 1:
        return [f"пневмоблок: {blocks or 'нет'}"]

    block = blocks[0]
    schema = "PneumoCCB" if block == "pneumo_ccb" else "PneumoE70"
    expected = {name: 0 for name in MESSAGES[schema].values()}
    expected.update(STARTUP[schema])
    actual = {name: 0 for name in expected}
    actual.update(state[block])
    for name, want in expected.items():
        if actual[name] != want:
            errors.append(f"{block}.{name}: {actual[name]}, ждём {want}")

    unknown = sorted(name for name in state if name.startswith("#"))
    if unknown:
        errors.append(f"лишние поля в пакете: {', '.join(unknown)}")

    for group in ("sb1", "hb1", "sb2", "hb2"):
        for name, value in state.get(group, {}).items():
            if value != 0:
                errors.append(f"{group}.{name}: {value}, ждём 0")
    return errors


GROUPS = (
    ("Пневмо CCB", "pneumo_ccb", "PneumoCCB"),
    ("Пневмо E-70", "pneumo_e70", "PneumoE70"),
    ("SB1", "sb1", "Sb1"),
    ("HB1", "hb1", "Hb1"),
    ("SB2", "sb2", "Sb2"),
    ("HB2", "hb2", "Hb2"),
)

SELECTOR_152 = {0: "O", 1: "I"}
SELECTOR_154 = {0: "NORM", 1: "I", 2: "I+II", 3: "II"}
SELECTOR_160 = {0: "I", 1: "O"}
SELECTOR_237 = {0: "I", 1: "O"}
E70_PAN = {0: "AUTO", 1: "I", 2: "II"}
CCB_PAN = {0: "P1", 1: "P2", 2: "AUTO"}


def field_names(schema_name):
    names = []
    for spec in MESSAGES[schema_name].values():
        if isinstance(spec, str):
            names.append(spec)
    return names


def describe(group, name, value):
    if name == "sb1_failure_mode_operation":
        return SELECTOR_152.get(value, str(value))
    if name == "sb1_bogie_cut_out":
        return SELECTOR_154.get(value, str(value))
    if name == "sb1_configuration_switch":
        return SELECTOR_160.get(value, str(value))
    if name == "sb1_vigilance_device_cut_off":
        return SELECTOR_237.get(value, str(value))
    if name == "pan_sel_sw" and group == "pneumo_e70":
        return E70_PAN.get(value, str(value))
    if name == "pan_sel_sw":
        return CCB_PAN.get(value, str(value))
    return "true" if value else "false"


class ReceiverWindow:
    def __init__(self, address):
        self._address = address
        self._stop = threading.Event()
        self._queue = queue.Queue()
        self._previous = {}
        self._rows = {}

        self._root = tk.Tk()
        self._root.title("WAG-9 FS")
        self._root.geometry("640x820")
        self._status = tk.StringVar(value=f"жду пакет с {address}")
        ttk.Label(self._root, textvariable=self._status, padding=(8, 6)).pack(fill=tk.X)

        frame = ttk.Frame(self._root)
        frame.pack(fill=tk.BOTH, expand=True)
        self._tree = ttk.Treeview(frame, columns=("state",), show="tree headings")
        self._tree.heading("#0", text="Орган")
        self._tree.heading("state", text="Состояние")
        self._tree.column("#0", width=360, stretch=True)
        self._tree.column("state", width=180, stretch=False, anchor=tk.W)
        scroll = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=self._tree.yview)
        self._tree.configure(yscrollcommand=scroll.set)
        self._tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self._tree.tag_configure("changed", background="#fff3bf")
        self._tree.tag_configure("absent", foreground="#888888")

        self._build_rows()
        self._root.protocol("WM_DELETE_WINDOW", self._close)
        threading.Thread(target=self._receive, name="fs-receiver", daemon=True).start()
        self._root.after(100, self._drain)

    def _build_rows(self):
        for title, key, schema in GROUPS:
            parent = self._tree.insert("", tk.END, text=title, open=True)
            for name in field_names(schema):
                item = self._tree.insert(parent, tk.END, text=name, values=("—",))
                self._rows[(key, name)] = item

    def _close(self):
        self._stop.set()
        self._root.destroy()

    def _receive(self):
        try:
            listen(self._address, False, self._queue.put, self._stop)
        except Exception as error:
            self._queue.put(error)

    def _drain(self):
        latest = None
        error = None
        while True:
            try:
                item = self._queue.get_nowait()
            except queue.Empty:
                break
            if isinstance(item, Exception):
                error = item
            else:
                latest = item
        if error is not None:
            self._status.set(str(error))
        elif latest is not None:
            self._show(*latest)
        if not self._stop.is_set():
            self._root.after(100, self._drain)

    def _show(self, vehicle, state):
        active = "E-70" if "pneumo_e70" in state else "CCB" if "pneumo_ccb" in state else "нет пневмоблока"
        self._status.set(f"{vehicle or '—'}   {active}   {time.strftime('%H:%M:%S')}")
        current = {}
        for title, key, schema in GROUPS:
            block = state.get(key)
            present = block is not None
            for name in field_names(schema):
                value = 0 if block is None else block.get(name, 0)
                path = (key, name)
                current[path] = value
                text = describe(key, name, value) if present else "нет в пакете"
                tags = ()
                if not present:
                    tags = ("absent",)
                elif self._previous.get(path) not in (None, value):
                    tags = ("changed",)
                self._tree.item(self._rows[path], values=(text,), tags=tags)
        self._previous = current

    def run(self):
        self._root.mainloop()


class PeerTimeout(Exception):
    pass


class ZmtpClient:
    """Минимальный ZMQ_CLIENT: рукопожатие NULL и кадры ZMTP 3.0."""

    def __init__(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self._buf = b""

    def connect(self, address, timeout=0.2):
        host, port = address.removeprefix("tcp://").rsplit(":", 1)
        self._sock.settimeout(1)
        self._sock.connect((host, int(port)))
        self._handshake()
        self._sock.settimeout(timeout)

    def close(self):
        self._sock.close()

    def send(self, payload):
        if len(payload) > 255:
            header = bytes([2]) + len(payload).to_bytes(8, "big")
        else:
            header = bytes([0, len(payload)])
        self._sock.sendall(header + payload)

    def recv(self):
        while True:
            flags, body = self._frame()
            if flags & 4:
                self._command(body)
                continue
            return body

    def _handshake(self):
        # Длина в подписи — как у libzmq: routing id пустой, поэтому 1.
        self._sock.sendall(b"\xff" + (1).to_bytes(8, "big") + b"\x7f")
        peer = self._exact(10)
        if peer[0] != 0xFF or not peer[9] & 1:
            raise RuntimeError("это не рукопожатие ZMTP")
        self._sock.sendall(b"\x03")
        major = self._exact(1)
        if major != b"\x03":
            raise RuntimeError("станция ответила не ZMTP 3")
        self._sock.sendall(b"\x00NULL" + bytes(16) + bytes(32))
        rest = self._exact(53)
        if rest[1:5] != b"NULL":
            raise RuntimeError("станция не использует механизм NULL")
        ready = b"\x05READY\x0bSocket-Type" + (6).to_bytes(4, "big") + b"CLIENT"
        self._sock.sendall(bytes([4, len(ready)]) + ready)
        while True:
            flags, body = self._frame()
            if flags & 4 and body.startswith(b"\x05READY"):
                return
            if not flags & 4:
                self._buf = bytes([flags, len(body)]) + body + self._buf

    def _command(self, body):
        if body.startswith(b"\x04PING") and len(body) >= 7:
            pong = b"\x04PONG" + body[5:]
            self._sock.sendall(bytes([4, len(pong)]) + pong)

    def _frame(self):
        # Кадр снимается с буфера только целиком: таймаут посреди кадра
        # не должен съесть заголовок.
        while True:
            buf = self._buf
            if len(buf) >= 2:
                flags = buf[0]
                if flags & 2:
                    start = 9
                    size = int.from_bytes(buf[1:9], "big") if len(buf) >= 9 else None
                else:
                    start = 2
                    size = buf[1]
                if size is not None and len(buf) >= start + size:
                    self._buf = buf[start + size:]
                    return flags, buf[start:start + size]
            self._fill()

    def _exact(self, size):
        while len(self._buf) < size:
            self._fill()
        data, self._buf = self._buf[:size], self._buf[size:]
        return data

    def _fill(self):
        try:
            chunk = self._sock.recv(65536)
        except socket.timeout as error:
            raise PeerTimeout from error
        if not chunk:
            raise ConnectionError("станция закрыла соединение")
        self._buf += chunk


def listen(address, check, on_packet=None, stop=None):
    client = ZmtpClient()
    try:
        client.connect(address)
    except (ConnectionError, OSError, PeerTimeout) as error:
        message = f"окно wag9_fs не слушает {address}"
        if on_packet is None:
            print(message, flush=True)
            return 1
        raise RuntimeError(message) from error

    if on_packet is None:
        print(f"слушаю {address}", flush=True)
    previous = None
    sent_at = 0.0
    deadline = time.monotonic() + 5

    try:
        return pump(client, check, on_packet, stop, previous, sent_at, deadline)
    finally:
        client.close()


def pump(client, check, on_packet, stop, previous, sent_at, deadline):
    while stop is None or not stop.is_set():
        now = time.monotonic()
        if now - sent_at >= KEEPALIVE_S:
            try:
                client.send(INIT)
            except (ConnectionError, OSError, PeerTimeout):
                message = "сервер не принял сообщение"
                if on_packet is None:
                    print(message, flush=True)
                    return 1
                raise RuntimeError(message)
            sent_at = now

        try:
            frame = client.recv()
        except PeerTimeout:
            if check and time.monotonic() > deadline:
                print("пакет не пришёл. Окно wag9_fs запущено?")
                return 1
            continue

        vehicle, state = unwrap(frame)
        if on_packet is not None:
            on_packet((vehicle, state))
            continue
        current = flatten(state)
        if previous is None:
            print(f"vehicle {vehicle}")
            for path in sorted(current):
                if current[path] != 0 and current[path] != "{}":
                    print(f"  {path} = {current[path]}")
            if not any(item != 0 and item != "{}" for item in current.values()):
                print("  все поля нулевые")
            if check:
                errors = startup_errors(state)
                if errors:
                    print("старт не совпал:")
                    for error in errors:
                        print(f"  {error}")
                    return 1
                print("старт совпал с нормальным поездом")
                return 0
        else:
            for path in sorted(set(previous) | set(current)):
                old = previous.get(path, 0)
                new = current.get(path, 0)
                if old != new and new != "{}" and old != "{}":
                    print(f"{path}: {old} -> {new}")
        previous = current


def main():
    parser = argparse.ArgumentParser(description="Клиент исходящего пакета WAG-9 FS")
    parser.add_argument("--addr", default=ADDRESS)
    parser.add_argument("--check", action="store_true", help="сверить один стартовый пакет и выйти")
    args = parser.parse_args()
    try:
        if args.check:
            return listen(args.addr, True)
        ReceiverWindow(args.addr).run()
        return 0
    except KeyboardInterrupt:
        return 0


if __name__ == "__main__":
    sys.exit(main())
