#include "Wag9FsInfoPanel.h"
#include <QFileInfo>

Wag9FsInfoPanel::Wag9FsInfoPanel(QWidget* parent)
    : QFrame(parent)
{
    ui.setupUi(this);
    connect(ui.closeButton, &QPushButton::clicked, this, &Wag9FsInfoPanel::closeRequested);
}

void Wag9FsInfoPanel::initialize(Wag9Zone zone, int refNumber, Wag9Side side, QAudioDeviceInfo* soundEffects)
{
    m_refNumber = refNumber;
    m_side = side;
    m_zone = zone;
    m_soundEffectsOutputDevice = soundEffects;

    buildUi(*ui.container);

    ui.debugPanel->setText(ui.debugPanel->text()
        .arg(toString(m_zone)).arg(m_refNumber)
        .arg(toString(m_side)));
    ui.debugPanel->raise();

    m_soundEffectsPrimary = new QSoundEffect(*m_soundEffectsOutputDevice, this);
    m_soundEffectsPrimary->setLoopCount(1);
    m_soundEffectsPrimary->setVolume(1.0f);

    m_soundEffectsSecondary = new QSoundEffect(*m_soundEffectsOutputDevice, this);
    m_soundEffectsSecondary->setLoopCount(1);
    m_soundEffectsSecondary->setVolume(1.0f);
}

void Wag9FsInfoPanel::setNumberTitle(const QString& title)
{
    ui.number->setText(title);
}

void Wag9FsInfoPanel::setTitleText(const QString& text)
{
    ui.title->setText(text);
}

void Wag9FsInfoPanel::setDebugPanelVisible(bool visible)
{
    ui.debugPanel->setVisible(visible);
}

PanelsHasher::argument_type Wag9FsInfoPanel::key() const
{
    return std::make_tuple(zone(), refNumber(), side());
}

QString Wag9FsInfoPanel::vehicleId() const
{
    return m_vehicleId;
}

void Wag9FsInfoPanel::setVehicleId(QString const& vehicleId)
{
    m_vehicleId = vehicleId;
}

int Wag9FsInfoPanel::refNumber() const
{
    return m_refNumber;
}

Wag9Side Wag9FsInfoPanel::side() const
{
    return m_side;
}

Wag9Zone Wag9FsInfoPanel::zone() const
{
    return m_zone;
}

void Wag9FsInfoPanel::setBackground(QWidget& container, const QString& url)
{
    container.setStyleSheet(
        QStringLiteral("border-image: url(\"%1\") 0 0 0 0 no-repeat no-repeat;").arg(url));
}

std::unique_ptr<KeySwitchButton>
Wag9FsInfoPanel::createSwitchButtonSymmetry(QStringList animationOnList,
    const QRect& controlButtonGeometry)
{
    QStringList animationOffList;
    for (auto rit = animationOnList.rbegin(); rit != animationOnList.rend(); ++rit)
        animationOffList << *rit;

    return createSwitchButton(animationOnList, animationOffList, controlButtonGeometry);
}

std::unique_ptr<KeySwitchButton> Wag9FsInfoPanel::createSwitchButton(
    QStringList animationOnList, QStringList animationOffList, const QRect& controlButtonGeometry)
{
    auto boxButton = std::make_unique<KeySwitchButton>(this);
    boxButton->setAnimationOnList(animationOnList);
    boxButton->setAnimationOffList(animationOffList);
    boxButton->setObjectName(QFileInfo(animationOnList.first()).baseName());
    boxButton->setGeometry(controlButtonGeometry);
    boxButton->setControlButtonGeometry(boxButton->rect());
    boxButton->setSilent(true);
    boxButton->init();

    return std::move(boxButton);
}

std::unique_ptr<QPushButton>
Wag9FsInfoPanel::createLamp(const QRect& geometry,
    const QString& offPixmap, const QString& onPixmap)
{
    QIcon icon;
    icon.addFile(offPixmap, QSize(), QIcon::Disabled, QIcon::Off);
    icon.addFile(onPixmap, QSize(), QIcon::Disabled, QIcon::On);

    return std::move(createLamp(geometry, icon));
}

std::unique_ptr<QPushButton>
Wag9FsInfoPanel::createLamp(const QRect& geometry, const QIcon& icon)
{
    auto button = std::make_unique<QPushButton>(this);
    button->setGeometry(geometry);
    button->setCheckable(true);
    button->setIcon(icon);
    button->setIconSize(button->geometry().size());
    button->setDisabled(true);

    return std::move(button);
}

std::unique_ptr<QPushButton>
Wag9FsInfoPanel::createSimpleButton(const QRect& geometry, const QIcon& icon, bool checked)
{
    auto lamp = createLamp(geometry, icon);
    lamp->setDisabled(false);
    lamp->setChecked(checked);

    return std::move(lamp);
}

std::unique_ptr<QPushButton> Wag9FsInfoPanel::createSimpleButton(const QRect& geometry,
    const QString& offPixmap, const QString& onPixmap, bool checked)
{
    auto lamp = createLamp(geometry, offPixmap, onPixmap);
    lamp->setDisabled(false);
    lamp->setChecked(checked);

    return std::move(lamp);
}

std::unique_ptr<QLabel> Wag9FsInfoPanel::createStaticLabel(const QString& text,
    const QRect& geometry)
{
    auto label = std::make_unique<QLabel>(
        QStringLiteral("<p style=\"line-height:65%;\">%1</p>").arg(text), this);
    label->setGeometry(geometry);

    QFont font;
    font.setFamily(QString::fromUtf8("Oswald"));
    font.setPointSize(8);
    font.setBold(true);
    font.setStyleStrategy(QFont::PreferAntialias);
    label->setFont(font);
    label->setStyleSheet(QString::fromUtf8("color:white;"));
    label->setScaledContents(true);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    label->setWordWrap(true);

    return label;
}

std::unique_ptr<HitachiButton> Wag9FsInfoPanel::createPressButton(const QRect& geometry,
    const QString& releasedPixmap, const QString& pressedPixmap)
{
    auto button = std::make_unique<HitachiButton>(this);
    button->setProperty("Off", releasedPixmap);
    button->setProperty("PressedOff", pressedPixmap);
    button->setGeometry(geometry);
    button->setState(ControlButtonState::Off);
    button->setSilent(true);
    button->raise();

    return std::move(button);
}

std::unique_ptr<HitachiButton> Wag9FsInfoPanel::createButton(const QRect& geometry,
    const QString& offPixmap, const QString& onPixmap, const QString& pressedOffPixmap,
    const QString& pressedOnPixmap)
{
    auto button = std::make_unique<HitachiButton>(this);
    button->setProperty("Off", offPixmap);
    button->setProperty("On", onPixmap);
    button->setProperty("PressedOff", pressedOffPixmap);
    button->setProperty("PressedOn", pressedOnPixmap);
    button->setGeometry(geometry);
    button->setState(ControlButtonState::Off);
    button->setSilent(true);

    return std::move(button);
}

std::unique_ptr<AutomatonButton>
Wag9FsInfoPanel::createAutomatonButton(const QRect& geometry,
    const QIcon& icon, const QString& name)
{
    auto button = std::make_unique<AutomatonButton>(this);
    button->setIcon(icon);
    button->setIconSize(geometry.size());
    button->setGeometry(geometry);
    button->setObjectName(name);
    button->setChecked(true);
    button->setSilentMode(false);

    return std::move(button);
}

void Wag9FsInfoPanel::togglePrimarySoundEffect(const QString& source, bool play) const
{
    const auto url = QUrl::fromLocalFile(source);

    if (m_soundEffectsPrimary->isPlaying() == play && m_soundEffectsPrimary->source() == url)
    {
        return;
    }

    m_soundEffectsPrimary->stop();
    if (!play)
    {
        return;
    }

    m_soundEffectsPrimary->setSource(url);
    m_soundEffectsPrimary->setLoopCount(1);
    m_soundEffectsPrimary->setVolume(1.0f);
    m_soundEffectsPrimary->play();
}

void Wag9FsInfoPanel::toggleSecondarySoundEffect(const QString& source, bool play) const
{
    const auto url = QUrl::fromLocalFile(source);

    if (m_soundEffectsSecondary->isPlaying() == play && m_soundEffectsSecondary->source() == url)
    {
        return;
    }

    m_soundEffectsSecondary->stop();
    if (!play)
    {
        return;
    }

    m_soundEffectsSecondary->setSource(url);
    m_soundEffectsSecondary->setLoopCount(1);
    m_soundEffectsSecondary->setVolume(1.0f);
    m_soundEffectsSecondary->play();
}

void Wag9FsInfoPanel::playAirDropSound(bool play) const
{
    static const QString airDrop = QStringLiteral(":/Wag9/fs/sounds/1/air_drop.wav");
    toggleSecondarySoundEffect(airDrop, play);
}

void Wag9FsInfoPanel::playValveToggleSound(bool opened) const
{
    static const QString open = QStringLiteral(":/Wag9/fs/sounds/14-15-18-27/valve_open.wav");
    static const QString close = QStringLiteral(":/Wag9/fs/sounds/14-15-18-27/valve_close.wav");

    toggleSecondarySoundEffect(opened ? open : close);
}

void Wag9FsInfoPanel::playPressSound(bool down) const
{
    static const QString press = QStringLiteral(":/Wag9/fs/sounds/17/Button_press.wav");
    static const QString release = QStringLiteral(":/Wag9/fs/sounds/17/Button_release.wav");

    toggleSecondarySoundEffect(down ? press : release);
}


QString toString(Wag9Side side)
{
    return side == Wag9Side::Left ? QStringLiteral("left") : QStringLiteral("right");
}

QString toString(Wag9Zone zone)
{
    switch (zone)
    {
    case Wag9Zone::Pantograph1: return QStringLiteral("Pantograph1");
    case Wag9Zone::PT: return QStringLiteral("PT");
    case Wag9Zone::SB1: return QStringLiteral("SB1");
    case Wag9Zone::HB1: return QStringLiteral("HB1");
    case Wag9Zone::Pantograph2: return QStringLiteral("Pantograph2");
    case Wag9Zone::SB2: return QStringLiteral("SB2");
    case Wag9Zone::HB2: return QStringLiteral("HB2");
    }
    return QStringLiteral("?");
}
