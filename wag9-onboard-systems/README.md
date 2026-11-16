# WAG-9 Fault Station

Окно инструктора для электровоза WAG-9. Это программа `wag9_fs`: схема локомотива на весь экран, по которой включают пантографы, открывают шкафы и крутят краны пневмопанели. Состояние пока только рисуется локально. Сообщения симулятору уже обёрнуты в protobuf, но сами поля WAG-9 ещё пустые.

Окно фиксировано 1920×1080, без рамки Windows. Картинки лежат в ресурсах Qt и адресуются как `:/Wag9/...`.

## Сборка

Нужны 64-битный Visual Studio 2022 и уже собранные зависимости. Путь к ним передаётся как `DEPS_BUILD_DIR`.

```bat
cmake -S wag9-onboard-systems -B wag9-build -G "Visual Studio 17 2022" -A x64 -DDEPS_BUILD_DIR=C:/Neokon/RRC/3rdParty
cmake --build wag9-build --config Debug --target wag9_fs
```

Стартовый проект в решении — `wag9_fs`. Готовый exe: `wag9-build\Debug\wag9_fs.exe`. Рядом с ним `CopyDependencies` кладёт `Wag9.xml`, Qt-плагины и переводы. Настройки программа читает из файла рядом с exe, а не из `data\` в исходниках.

Новый `.cpp` в `src/wag9/CMakeLists.txt` требует повторного запуска CMake. Новая картинка в `res/Wag9.qrc` подхватывается обычной пересборкой.

## Как устроены каталоги

| Путь | Зачем |
| --- | --- |
| `src/wag9/` | Само окно FS, сервер, пневмопанель |
| `src/` | Библиотека `common`: кнопки, ZMQ, имена файлов |
| `ui/Wag9Fs.ui` | Разметка главного окна |
| `res/Wag9.qrc` и `res/fs/` | Картинки, которые попадают в exe |
| `data/Wag9.xml` | Положение окна и адрес ZMQ |
| `src/neokon-shared/proto/Wag9FaultStation.proto` | Пустые сообщения WAG-9 |
| `translations/` | `app_en.ts`, `app_ru.ts`, `app_lv.ts`. Без них цель `CompileTranslations` не собирается |

`common` — это статическая библиотека из виджетов в `src/`, а не отдельная папка. `neokon-shared` — общий код поездов. Логика WAG-9 там не живёт, кроме proto-файла.

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

`Q_INIT_RESOURCE(Wag9)` требует, чтобы файл ресурсов назывался `Wag9.qrc`. Префикс внутри него — `/Wag9`, поэтому картинка `fs/Background.png` открывается как `:/Wag9/fs/Background.png`.

`FileNames::Wag9Settings()` возвращает `Wag9.xml`, `FileNames::Wag9LogFileName()` — `Wag9-log.txt`. Оба файла ищутся в каталоге exe.

Дальше `Wag9Fs::init` собирает интерфейс:

```cpp
ui.setupUi(this);
setWindowFlag(Qt::FramelessWindowHint);
setWindowFlag(Qt::WindowStaysOnTopHint, m_staticConfig->fsSettings().topMost);
move(m_staticConfig->fsSettings().position);
prepareZones();
createPantographs();
m_pneumoPanel = new Wag9PneumoPanel(this);
m_pneumoPanel->hide();
ui.background->lower();
ui.hintsButton->raise();
createServer();
```

`FramelessWindowHint` убирает заголовок Windows. Иначе клиентская область 1920×1080 вместе с заголовком не помещается на экран 1080 и низ картинки обрезается. Положение берётся из XML, размер окна XML не меняет.

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

`Wag9Configuration` читает узел `FS` в `m_fsSettings`. Узлы панелей и контроллера в этом файле не обязательны: для них остаются значения по умолчанию.

## Главное окно

`ui/Wag9Fs.ui` — форма `Wag9Fs` размером 1920×1080. Фон — это `QLabel` `background` с картинкой `:/Wag9/fs/Background.png`, а не `border-image`. Так поверх фона можно класть другие виджеты, не затирая их стилем.

Все `QPushButton` прозрачные и без рамки. Синяя подсветка наведения и нажатия не рисуется: голубые прямоугольники на исходных картинках только показывают границы кнопок.

Зоны — прозрачные кнопки. У шкафов имя начинается с `link_`, у пантографов с `pan_`. У каждой кнопки есть свойства `zone`, `side` и, для шкафа, `frame` — путь к картинке, которую надо открыть.

Адреса зон заданы в `Wag9Vehicle.h`. Это стороны и места на локомотиве, а не вагоны Class 170.

```cpp
enum class Wag9Side { Left, Right };

enum class Wag9Zone
{
    Pantograph1,
    PT,
    SB1,
    HB1,
    Pantograph2,
    SB2,
    HB2,
};
```

На правом виде слева направо: пантограф 1, PT, SB1, HB1, пантограф 2. На левом: пантограф 1, SB2, HB2, пантограф 2. Физических пантографов два, каждый нарисован на обеих сторонах. Клик двигает только тот рисунок, по которому попали.

## Шкафы

`prepareZones` находит кнопки `link_*` и по клику открывает кадр. Повторный клик по той же зоне или клик по другой зоне закрывает предыдущую.

```cpp
if (button == ui.link_PT)
{
    showPneumo();
}
else
{
    showFrame(button->property("frame").toString());
}
```

`showFrame` кладёт картинку шкафа в `frameView` по центру. Если она больше 90% окна, уменьшает с сохранением пропорций. Вокруг остаётся затемнение `rgba(0,0,0,140)`.

SB1, SB2, HB1 и HB2 — одна готовая картинка на шкаф. PT так не открывается: для него вызывается `showPneumo`.

Закрытие — клик по затемнению вне кадра. `eventFilter` на `frameView` сравнивает точку с `m_frameRect`. Красной кнопки закрытия нет.

```cpp
if (!m_frameRect.contains(mouseEvent->pos()))
{
    closeFrame();
    return true;
}
```

`closeFrame` снимает отметку со всех `link_*`, прячет `frameView` и пневмопанель.

## Подсказки

Кнопка `hintsButton` стоит в правом нижнем углу, 1804×1032, размер 108×40. Выключенная — белая с синим текстом `#1c8fd4`. Нажатая — синяя с белым текстом. Это только стиль самой кнопки.

Надписи PT, SB1, HB1, SB2 и HB2 — отдельные `QLabel` `hint_*`. В них лежат вырезанные плашки `res/fs/hints/*.png`, те же пиксели, что на исходной картинке с подписями. Пока кнопка не нажата, подписи скрыты.

```cpp
connect(ui.hintsButton, &QPushButton::toggled, this, [this, hintLabels](bool enabled)
{
    for (QLabel* label : hintLabels)
    {
        label->setVisible(enabled);
        if (enabled)
        {
            label->raise();
        }
    }
});
```

Фон при этом не меняется и остаётся `Background.png`. У подписей стоит `WA_TransparentForMouseEvents`, поэтому клик по надписи проходит в зону шкафа под ней. Если в этот момент открыт шкаф, `frameView` снова поднимается наверх и подписи не рисуются поверх затемнения.

## Пантографы

Кадры: `res/fs/panto/left/01.png` … `10.png` и то же для `right`. Кадр 01 — опущен, кадр 10 — поднят. Размер кадра 212×166. `left` — пантограф 1, рычаг уходит вправо. `right` — пантограф 2, рычаг уходит влево.

`createPantographs` на каждое место клика кладёт `KeySwitchButton` того же размера. Список подъёма — кадры 01…10, список опускания — тот же список задом наперёд. Кнопка видна, но сама не нажимается: клик принимает прозрачная кнопка поверх неё и переключает состояние.

```cpp
view->setAnimationOnList(frames);
view->setAnimationOffList(framesDown);
view->setSilent(true);
view->init();
view->setClickable(false);

connect(clickArea, &QPushButton::clicked, view, [view]
{
    if (view->animationState() != KeySwitchButton::AnimationState::None)
    {
        return;
    }
    view->setState(view->state() == ControlButtonState::On
        ? ControlButtonState::Off
        : ControlButtonState::On);
});
```

`init()` сразу показывает первый кадр списка подъёма, то есть опущенный пантограф. Пока идёт анимация, повторный клик игнорируется. Звук выключен.

Четыре рисунка независимы. `pan_right_1` и `pan_left_1` используют одни и те же кадры «левого» пантографа, но у каждого свой `KeySwitchButton`. Клик по одному не двигает второй.

На фоне пантографа быть не должно. Кадры прозрачные и рисуются поверх крыши. Если пантограф нарисован ещё и на `Background.png`, после подъёма он остаётся виден в прозрачных местах кадра.

## Пневмопанель

`Wag9PneumoPanel` — виджет 1000×1000. Фон `res/fs/pneumo/Background.png` — пустая панель. Краны — отдельные спрайты поверх неё, каждый в положении с кадра `Pneumo1_Frame.png`.

`showPneumo` центрирует этот виджет и оставляет вокруг то же затемнение. Клик по затемнению закрывает панель, клик по самому виджету — нет.

Кран — маленький `QLabel`. Нажатие сменяет картинку на следующую и по кругу возвращается к первой.

```cpp
void mousePressEvent(QMouseEvent* event) override
{
    m_index = (m_index + 1) % m_frames.size();
    setPixmap(QPixmap(m_frames.at(m_index)));
    event->accept();
}
```

Одинаковые краны делят один набор кадров, но у каждого свой индекс. Клик двигает только тот кран, по которому попали.

| Орган | Спрайт | Кадров | Стартовый кадр |
| --- | --- | --- | --- |
| EC-COC, TC2-COC, TC1-COC | `SwitchBlack2` | 2 | 1 |
| 21(SIFA)-COC | `SwitchRed1` | 2 | 1 |
| FP-COC | `SwitchYellow1` | 2 | 1 |
| SAND1-COC, SAND2-COC | `SwitchBlack3` | 2 | 1 |
| UL-COC, CB-COC, PAN1.COC, PAN2.COC | `SwitchBlack2` | 2 | 2 |
| PANS COC | `SwitchKey` | 2 | 1 |
| PAN-SEL-SW | `SwitchBlack1` | 3 | 1 |
| PASS/GOODS | `SwitchBlack4` | 2 | 1 |
| PER-COC | `SwitchRed1` | 2 | 1 |
| DEAD ENGINE | `SwitchYellow2` | 2 | 2 |
| G/P | `SwitchYellow3` | 2 | 1 |

Имена файлов не совпадают с цветом органа: `SwitchKey` — синий рычаг PANS, `SwitchYellow1` — серый FP. Соответствие снято наложением спрайта на кадр с исходным положением.

Стрелка манометра — `ArrowBlack1.png`, один кадр, клики сквозь неё проходят. Шесть ножевых рубильников в среднем ряду нарисованы на фоне и не переключаются. Положения органов в симулятор пока не отправляются.

## Связь с симулятором

`createServer` открывает `Wag9FsServer` на адресе из `Wag9.xml`. Таймер раз в 100 мс вызывает `reportStationState`. Сейчас эта функция пустая.

Входящий пакет сначала пробуют разобрать как `FaultStationInitMessage`. Если в нём есть хотя бы один номер вагона, это инициализация. Иначе пакет читается как `TrainSimToFaultStationMessage`, а из поля `to_fault_station_bytes` достаётся `proto::wag9::fs::FaultStationIncomingMessage`.

```cpp
proto::FaultStationInitMessage initMsg;
if (initMsg.ParseFromArray(zmqMsg.data(), zmqMsg.size())
    && initMsg.vehicle_numbers_size() > 0)
{
    emit initMessageReceived(initMsg);
}
```

Исходящее сообщение заворачивается так:

```cpp
proto::TrainSimFromFaultStationMessage wrapper;
wrapper.set_vehicle_number("WAG9");
wrapper.set_from_fault_station_bytes(msg.SerializeAsString());
```

Пакет `proto.wag9.fs` в C++ называется `proto::wag9::fs`, не `proto::Wag9::fs`. Оба сообщения WAG-9 пока без полей:

```protobuf
syntax = "proto3";
package proto.wag9.fs;

message FaultStationIncomingMessage
{
}

message FaultStationOutgoingMessage
{
}
```

Если от симулятора 10 секунд нет пакетов, сервер забывает `routingId` и шлёт `disconnected`. Пока клиент не подключился, `sendMessage` ничего не отправляет.

## Что ещё не подключено

`Wag9::PanelFactory::create` для любой зоны возвращает `nullptr`. `Wag9FsInfoPanel` — заготовка панели с ключом `(зона, номер, сторона)` и пустыми `writeStateToProto` / `processIncomingMessage`. Главное окно её не показывает: шкафы открываются картинкой или `Wag9PneumoPanel`.

`Wag9Panels.cpp` пустой. В библиотеку `neokon` по-прежнему входит `Class170VarsToVirtualControlViewer`: это общий файл, к окну WAG-9 он не относится.
