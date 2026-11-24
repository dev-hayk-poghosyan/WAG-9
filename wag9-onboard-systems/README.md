# WAG-9 Fault Station

Окно машинного отделения электровоза WAG-9. Это программа `wag9_fs`: схема локомотива на весь экран, по которой открывают шкафы SB1, HB1, SB2, HB2 и крутят краны пневмопанели CCB или E-70.

Раз в 100 мс окно отправляет симулятору положения всех органов, которыми управляет обучаемый. Всё, что делает сам локомотив, приходит из симулятора: давление для манометров, цвета ламп E-70, положение пантографов. Окно показывает присланное как есть, без своей логики.

Окно фиксировано 1920×1080, без рамки Windows. Картинки лежат в ресурсах Qt и адресуются как `:/Wag9/...`.

## Сборка

Нужны 64-битный Visual Studio 2022 и уже собранные зависимости. Путь к ним передаётся как `DEPS_BUILD_DIR`.

```bat
cmake -S wag9-onboard-systems -B wag9-build -G "Visual Studio 17 2022" -A x64 -DDEPS_BUILD_DIR=C:/Neokon/RRC/3rdParty
cmake --build wag9-build --config Debug --target wag9_fs
```

Конфигураций две: `Debug` и `RelWithDebInfo`. Стартовый проект в решении — `wag9_fs`. Готовый exe: `wag9-build\Debug\wag9_fs.exe`. Рядом с ним `CopyDependencies` кладёт `Wag9.xml`, Qt-плагины и переводы. Настройки программа читает из файла рядом с exe, а не из `data\` в исходниках.

Новый `.cpp` в `src/wag9/CMakeLists.txt` требует повторного запуска CMake. Новая картинка в `res/Wag9.qrc` подхватывается обычной пересборкой.

## Каталоги

| Путь | Зачем |
| --- | --- |
| `src/wag9/` | Окно, панели, сервер |
| `src/` | Библиотека `common`: кнопки, ZMQ, имена файлов |
| `ui/Wag9Fs.ui` | Разметка главного окна |
| `res/Wag9.qrc` и `res/fs/` | Картинки, которые попадают в exe |
| `data/Wag9.xml` | Положение окна и адрес ZMQ |
| `src/neokon-shared/proto/Wag9FaultStation.proto` | Сообщения WAG-9 |
| `tools/` | Тестеры на Python вместо симулятора |
| `translations/` | `app_en.ts`, `app_ru.ts`, `app_lv.ts`. Без них цель `CompileTranslations` не собирается |

`common` — статическая библиотека из виджетов в `src/`, а не отдельная папка. `neokon-shared` — общий код поездов. Логика WAG-9 там не живёт, кроме proto-файла.

## Файлы в `src/wag9/`

| Файл | Что внутри |
| --- | --- |
| `FaultStationMain.cpp` | `main`: Qt, лог, конфиг, окно |
| `Wag9Fs` | Главное окно: зоны, попапы, пантографы, тормозная система, обмен с симулятором |
| `Wag9CubiclePanel` | Шкафы SB1, HB1, SB2, HB2 |
| `Wag9PneumoPanel` | Пневмопанель CCB |
| `Wag9PneumoE70Panel` | Пневмопанель E-70 с лампами |
| `Wag9SpriteSwitch` | Орган из нескольких кадров: кран, тумблер, автомат, кнопка с лампой |
| `Wag9Needle` | Стрелка манометра |
| `Wag9FsServer` | ZMQ-сервер, обёртки сообщений |
| `Wag9Configuration` | Чтение `Wag9.xml` |

## Старт

`FaultStationMain.cpp` поднимает Qt, лог и окно.

```cpp
Q_INIT_RESOURCE(Wag9);

QApplication app(argc, argv);
const QString configPath = appDir.filePath(FileNames::Wag9Settings());
staticConfig = loadWag9Configuration(configPath);

Wag9Fs fs{};
fs.init(staticConfig);
fs.show();
return app.exec();
```

`Q_INIT_RESOURCE(Wag9)` требует, чтобы файл ресурсов назывался `Wag9.qrc`. Префикс внутри него — `/Wag9`, поэтому `res/fs/panto/left/01.png` открывается как `:/Wag9/fs/panto/left/01.png`.

`FileNames::Wag9Settings()` возвращает `Wag9.xml`, `FileNames::Wag9LogFileName()` — `Wag9-log.txt`. Оба файла ищутся в каталоге exe.

`Wag9Fs::init` собирает окно: зоны, пантографы, шесть попапов (две пневмопанели и четыре шкафа, все скрыты) и сервер.

`FramelessWindowHint` убирает заголовок Windows. Иначе клиентская область 1920×1080 вместе с заголовком не помещается на экран 1080 и низ картинки обрезается. Положение окна берётся из XML, размер XML не меняет.

`data/Wag9.xml`:

```xml
<FS>
    <Geometry left="0" top="0"/>
    <Debug>False</Debug>
    <TopMost>False</TopMost>
    <Network>
        <BindAddr>tcp://127.0.0.1:12282</BindAddr>
    </Network>
</FS>
```

## Главное окно

Фон — `QLabel` `background`. Картинка зависит от тормозной системы: `WAG9_FS_Background_CCB.png` или `WAG9_FS_Background_E70.png`. Поверх фона лежат прозрачные кнопки без рамки:

| Кнопки | Что делают |
| --- | --- |
| `link_PT`, `link_SB1`, `link_HB1`, `link_SB2`, `link_HB2` | Открывают попап зоны. Зона берётся из свойства `zone` |
| `pan_right_1`, `pan_left_1`, `pan_right_2`, `pan_left_2` | Только задают место рисунка пантографа, сами скрыты |
| `pneumoCcb`, `pneumoE70` | Выбор тормозной системы, левый нижний угол |
| `hintsButton` | Подписи зон, правый нижний угол |

## Попапы

`m_popups` хранит панели по ключу зоны: `PT` — CCB, `E70` — E-70, `SB1`, `HB1`, `SB2`, `HB2` — шкафы. Обе пневмопанели дополнительно лежат в типизированных полях `m_ccbPanel` и `m_e70Panel`.

Клик по `link_*` открывает панель зоны по центру окна поверх затемнения `frameView` (`rgba(0,0,0,140)`). Для `link_PT` панель выбирает `getPneumoPanel()` по текущей тормозной системе. Повторный клик по той же зоне, клик по другой зоне или клик по затемнению вне панели закрывает текущую.

```cpp
if (!m_frameRect.contains(mouseEvent->pos()))
{
    closeFrame();
    return true;
}
```

Панели только прячутся, а не удаляются. Положения органов сохраняются между открытиями.

## Органы

Каждый орган — `Wag9SpriteSwitch`, `QLabel` с набором кадров `<sprite>_01.png` … `<sprite>_NN.png`. `objectName` органа совпадает с именем поля в proto, по нему `Wag9Fs` читает положение.

```cpp
place("ul_coc", "SwitchBlack2", {683, 136}, 2, 1);   // name, sprite, position, frames, initial frame
```

Режимы нажатия (`SwitchActuation`):

- `Cycle` — клик переводит на следующий кадр, после последнего возвращается к первому. `position()` — номер кадра с нуля.
- `Lamp` — кнопка с подсветкой (`sb1_configuration_button`). Пока кнопка нажата, показывается кадр нажатия. Отпускание над кнопкой переключает лампу. `position()` — 0 или 1.

`framesOf(folder, sprite, frameCount)` собирает список кадров из `:/Wag9/fs/<folder>/`.

## Шкафы

`Wag9CubiclePanel(zone)` берёт фон `cubicle/<zone>_Background.png` и кладёт органы своего шкафа. Ряды одинаковых автоматов задаются функцией `addRow`: спрайт, общая `y`, список `x` и список имён.

## Пневмопанели

**CCB** (`Wag9PneumoPanel`) — фон `pneumo/ccb/Background.png`, 17 органов, манометр. Координаты в `place` — левый верхний угол первого кадра.

| Орган | Спрайт | Кадров | Стартовый кадр |
| --- | --- | --- | --- |
| EC-COC, TC2-COC, TC1-COC | `SwitchBlack2` | 2 | 0 |
| 21(SIFA)-COC | `SwitchRed1` | 2 | 0 |
| FP-COC | `SwitchYellow1` | 2 | 0 |
| SAND1-COC, SAND2-COC | `SwitchBlack3` | 2 | 0 |
| UL-COC, CB-COC | `SwitchBlack2` | 2 | 1 |
| PANS COC | `SwitchKey` | 2 | 1 |
| PAN1.COC, PAN2.COC | `SwitchBlack2` | 2 | 0 |
| PAN-SEL-SW | `SwitchBlack1` | 3 | 0 |
| PASS/GOODS | `SwitchBlack4` | 2 | 0 |
| PER-COC | `SwitchRed1` | 2 | 0 |
| DEAD ENGINE | `SwitchYellow2` | 2 | 0 |
| G/P | `SwitchYellow3` | 2 | 0 |

Имена файлов не совпадают с цветом органа: `SwitchKey` — синий рычаг PANS, `SwitchYellow1` — серый FP.

**E-70** (`Wag9PneumoE70Panel`) — рамка `BaseFrame.png` и прокручиваемая картинка `Frame.png`, уменьшенная до ширины области прокрутки. Координаты в `place` даны в пикселях исходной `Frame.png` и умножаются на `m_scale`. Для рычагов, которые вращаются не вокруг центра кадра, передаётся точка оси `hinge`.

Лампы PROCESSOR FAULT, SYSTEM FAULT и PSU NORMAL — `QLabel` с именами `processor_fault`, `system_fault`, `psu_normal`. При старте все три погашены, цвет присылает симулятор. Цвет меняет `setLed(name, LedState)`.

**Манометр.** На обеих панелях стрелка — `Wag9Needle`. Шкала 0…10 кгс/см² на 270° по часовой (`kPantoGaugeMax`, `kPantoGaugeSweep` в `Wag9Needle.h`). Картинка стрелки в исходном виде указывает на ноль, ось — центр картинки. Значение задаёт `setPressure(value)` панели.

## Тормозная система

На локомотиве стоит либо CCB, либо E-70. Текущая система хранится в `m_brakeSystem` (`BrakeSystem::Ccb` по умолчанию). От неё зависят:

- какая пневмопанель открывается по `link_PT`;
- какой блок, `pneumo_ccb` или `pneumo_e70`, уходит в исходящем пакете;
- фон окна.

Систему переключают кнопки `pneumoCcb` и `pneumoE70`. `setBrakeSystem` ставит поле и отмечает нужную кнопку под `QSignalBlocker`, фон меняет `onPneumoModeToggled`.

Систему выбирают до запуска симулятора. Пока симулятор подключён, обе кнопки заблокированы: на работающем локомотиве оборудование не меняется. Блокировку ставит `setBrakeSystemLocked` по сигналам `connected` и `disconnected` сервера. Симулятор узнаёт систему по тому, какой блок пришёл в пакете: `pneumo_ccb` или `pneumo_e70`.

## Пантографы

Кадры: `res/fs/panto/left/01.png` … `10.png` и то же для `right`. Кадр 01 — опущен, кадр 10 — поднят. `left` — пантограф 1, `right` — пантограф 2.

Каждый пантограф нарисован дважды: на правом и на левом виде локомотива. На место каждой кнопки `pan_*` `addPantographView` кладёт `KeySwitchButton` с кадрами подъёма и теми же кадрами задом наперёд для опускания, а саму кнопку прячет. Кликать по пантографам нельзя: их поднимают из кабины.

Положение присылает симулятор в полях `pantograph_1_raised` и `pantograph_2_raised`. `setPantographPair` двигает оба вида одного пантографа вместе.

Пока идёт анимация (~0,6 с), пакеты для этого пантографа пропускаются. `KeySwitchButton` обновляет своё состояние только в конце анимации, поэтому `setState` посреди неё запустил бы анимацию заново, и при пакетах раз в 100 мс пантограф никогда бы не доехал. Если состояние сменилось во время анимации, его применит первый пакет после неё.

## Подсказки

`hintsButton` показывает и прячет `QLabel` `hint_*` с подписями PT, SB1, HB1, SB2, HB2. У подписей стоит `WA_TransparentForMouseEvents`, поэтому клик по надписи проходит в зону под ней. Если открыт попап, он остаётся поверх подписей.

## Связь с симулятором

`Wag9FsServer` — ZMQ-сокет `SERVER` на адресе из `Wag9.xml`. Клиентом первым подключается симулятор. Пока клиента нет, исходящие пакеты не отправляются.

**Входящие пакеты.** Сначала пакет разбирается как `TrainSimToFaultStationMessage`. Если в нём непустое поле `to_fault_station_bytes`, оттуда достаётся `FaultStationIncomingMessage`. Иначе пакет пробуют прочитать как `FaultStationInitMessage`. Порядок важен: поле 1 в обоих сообщениях строковое, и пакет с данными иначе разбирается как init.

```protobuf
message FaultStationIncomingMessage
{
    optional double ccb_manometer_pressure = 1;   // kgf/cm2
    optional double e70_manometer_pressure = 2;
    optional LedState e70_processor_fault = 3;
    optional LedState e70_system_fault = 4;
    optional LedState e70_psu_normal = 5;
    optional bool pantograph_1_raised = 6;
    optional bool pantograph_2_raised = 7;
}
```

Все поля `optional`: симулятор может слать только то, что изменилось, а `handleIncomingPackageFromSim` трогает только пришедшие поля.

**Исходящие пакеты.** Раз в 100 мс (`kReportIntervalMs`) `reportStationState` собирает `FaultStationOutgoingMessage`: блок активной пневмопанели и четыре шкафа. Поля 1 и 2 (пантографы) в нём зарезервированы: пантографы переехали во входящее сообщение. Функции `fill*` читают положение каждого органа по имени. Если орган не найден, в лог пишется `Missing switch <name>` и в пакет уходит 0.

```cpp
proto::TrainSimFromFaultStationMessage wrapper;
wrapper.set_vehicle_number("WAG9");
wrapper.set_from_fault_station_bytes(msg.SerializeAsString());
```

Если от симулятора 10 секунд нет пакетов, сервер забывает `routingId` и шлёт `disconnected`.

Пакет `proto.wag9.fs` в C++ называется `proto::wag9::fs`. Значения enum в proto3 живут в области пакета, поэтому у них префикс типа: `LED_STATE_GREEN`, `SELECTOR_154_I_PLUS_II`.

## Тестеры

Оба скрипта в `tools/` заменяют симулятор. Сначала нужно запустить `wag9_fs`. Скрипты говорят на ZMTP сами, без pyzmq: установленный pyzmq собран без сокетов `CLIENT`/`SERVER`.

```bat
python tools/fs_receive_from_server.py            :: window with every field of the outgoing packet
python tools/fs_receive_from_server.py --check    :: check one startup packet and exit

python tools/fs_send_to_server.py                 :: window: gauges, lamp colors, pantographs
python tools/fs_send_to_server.py --ccb 5 --e70 3.5 --psu-normal green
python tools/fs_send_to_server.py --pantograph-1 up
python tools/fs_send_to_server.py --sweep         :: swing needles, cycle lamps, raise/lower pantographs
```

`--check` сверяет стартовый пакет с таблицей `STARTUP` в `fs_receive_from_server.py`: поля, которые у исправного поезда не равны нулю. При смене стартового кадра органа таблицу нужно поправить.

Оба тестера можно держать открытыми одновременно.

## Что ещё не решено

- Кто шлёт тормозную систему и номер локомотива. Сейчас система выбирается кнопкой, номер всегда `WAG9`. `FaultStationInitMessage` разбирается, но никто его не отправляет.
- На E-70 органы `switch_1` и `switch_2` пока без настоящих имён.
