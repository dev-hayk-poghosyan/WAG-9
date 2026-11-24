#include "Wag9Fs.h"
#include "Wag9CubiclePanel.h"
#include "Wag9PneumoPanel.h"
#include "Wag9PneumoE70Panel.h"
#include "Wag9SpriteSwitch.h"

#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QStringList>
#include <QButtonGroup>

#include <algorithm>

namespace
{

const QString kPneumoCcb = QStringLiteral("PT");
const QString kPneumoE70 = QStringLiteral("E70");
const QString kSb1 = QStringLiteral("SB1");
const QString kHb1 = QStringLiteral("HB1");
const QString kSb2 = QStringLiteral("SB2");
const QString kHb2 = QStringLiteral("HB2");

constexpr int kReportIntervalMs = 100;
constexpr int kPantographFrameCount = 10;

const QString kBackgroundCcb = QStringLiteral(":/Wag9/fs/WAG9_FS_Background_CCB.png");
const QString kBackgroundE70 = QStringLiteral(":/Wag9/fs/WAG9_FS_Background_E70.png");

int positionOf(QWidget* panel, const char* name)
{
    if (!panel)
    {
        qWarning() << "Missing panel for" << name;
        return 0;
    }

    auto* sw = panel->findChild<Wag9SpriteSwitch*>(QLatin1String(name));
    if (!sw)
    {
        qWarning() << "Missing switch" << name;
        return 0;
    }
    return sw->position();
}

bool flagOf(QWidget* panel, const char* name)
{
    return positionOf(panel, name) != 0;
}

void fillPneumoCcb(proto::wag9::fs::PneumoCCB* ccb, QWidget* panel)
{
    ccb->set_ec_coc(flagOf(panel, "ec_coc"));
    ccb->set_tc2_coc(flagOf(panel, "tc2_coc"));
    ccb->set_tc1_coc(flagOf(panel, "tc1_coc"));
    ccb->set_sifa_coc(flagOf(panel, "sifa_coc"));
    ccb->set_fp_coc(flagOf(panel, "fp_coc"));
    ccb->set_sand1_coc(flagOf(panel, "sand1_coc"));
    ccb->set_sand2_coc(flagOf(panel, "sand2_coc"));
    ccb->set_ul_coc(flagOf(panel, "ul_coc"));
    ccb->set_cb_coc(flagOf(panel, "cb_coc"));
    ccb->set_pans_coc(flagOf(panel, "pans_coc"));
    ccb->set_pan1_coc(flagOf(panel, "pan1_coc"));
    ccb->set_pan2_coc(flagOf(panel, "pan2_coc"));
    ccb->set_pan_sel_sw(positionOf(panel, "pan_sel_sw"));
    ccb->set_pass_goods(flagOf(panel, "pass_goods"));
    ccb->set_per_coc(flagOf(panel, "per_coc"));
    ccb->set_dead_engine(flagOf(panel, "dead_engine"));
    ccb->set_gp(flagOf(panel, "gp"));
}

void fillPneumoE70(proto::wag9::fs::PneumoE70* e70, QWidget* panel)
{
    e70->set_pan_sel_sw(positionOf(panel, "pan_sel_sw"));
    e70->set_switch_1(flagOf(panel, "switch_1"));
    e70->set_switch_2(flagOf(panel, "switch_2"));
    e70->set_pan1(flagOf(panel, "pan1"));
    e70->set_pan2(flagOf(panel, "pan2"));
    e70->set_vcb(flagOf(panel, "vcb"));
    e70->set_pantog(flagOf(panel, "pantog"));
    e70->set_brake_feed_pipe(flagOf(panel, "brake_feed_pipe"));
    e70->set_dead_engine(flagOf(panel, "dead_engine"));
    e70->set_sand_1(flagOf(panel, "sand_1"));
    e70->set_sand_2(flagOf(panel, "sand_2"));
}

void fillSb1(proto::wag9::fs::Sb1* sb1, QWidget* panel)
{
    sb1->set_sb1_failure_mode_operation(
        static_cast<proto::wag9::fs::Selector152>(positionOf(panel, "sb1_failure_mode_operation")));
    sb1->set_sb1_bogie_cut_out(
        static_cast<proto::wag9::fs::Selector154>(positionOf(panel, "sb1_bogie_cut_out")));
    sb1->set_sb1_configuration_switch(
        static_cast<proto::wag9::fs::Selector160>(positionOf(panel, "sb1_configuration_switch")));
    sb1->set_sb1_vigilance_device_cut_off(
        static_cast<proto::wag9::fs::Selector237>(positionOf(panel, "sb1_vigilance_device_cut_off")));
    sb1->set_sb1_simulation(flagOf(panel, "sb1_simulation"));
    sb1->set_sb1_configuration_button(flagOf(panel, "sb1_configuration_button"));
    sb1->set_sb1_drivers_cab(flagOf(panel, "sb1_drivers_cab"));
    sb1->set_sb1_panto_vcb_control(flagOf(panel, "sb1_panto_vcb_control"));
    sb1->set_sb1_24v_48v_power_supply(flagOf(panel, "sb1_24v_48v_power_supply"));
    sb1->set_sb1_lighting_front(flagOf(panel, "sb1_lighting_front"));
    sb1->set_sb1_electronics_traction_converter_1(flagOf(panel, "sb1_electronics_traction_converter_1"));
    sb1->set_sb1_power_supply_gate_units(flagOf(panel, "sb1_power_supply_gate_units"));
    sb1->set_sb1_monitoring(flagOf(panel, "sb1_monitoring"));
    sb1->set_sb1_electronics_aux_converter_1(flagOf(panel, "sb1_electronics_aux_converter_1"));
    sb1->set_sb1_central_electronics_1(flagOf(panel, "sb1_central_electronics_1"));
    sb1->set_sb1_central_electronics_2(flagOf(panel, "sb1_central_electronics_2"));
    sb1->set_sb1_speed_sensor(flagOf(panel, "sb1_speed_sensor"));
}

void fillHb1(proto::wag9::fs::Hb1* hb1, QWidget* panel)
{
    hb1->set_hb1_transformer_oil_pump(flagOf(panel, "hb1_transformer_oil_pump"));
    hb1->set_hb1_converter_oil_pump(flagOf(panel, "hb1_converter_oil_pump"));
    hb1->set_hb1_main_compressor(flagOf(panel, "hb1_main_compressor"));
    hb1->set_hb1_traction_motor_blower(flagOf(panel, "hb1_traction_motor_blower"));
    hb1->set_hb1_scavenge_blower_tm(flagOf(panel, "hb1_scavenge_blower_tm"));
    hb1->set_hb1_vent_oil_cooler(flagOf(panel, "hb1_vent_oil_cooler"));
    hb1->set_hb1_mrb1(flagOf(panel, "hb1_mrb1"));
    hb1->set_hb1_scavenge_blower_mr(flagOf(panel, "hb1_scavenge_blower_mr"));
    hb1->set_hb1_cab_ventilation(flagOf(panel, "hb1_cab_ventilation"));
    hb1->set_hb1_cab_heater(flagOf(panel, "hb1_cab_heater"));
    hb1->set_hb1_crew_fan(flagOf(panel, "hb1_crew_fan"));
}

void fillSb2(proto::wag9::fs::Sb2* sb2, QWidget* panel)
{
    sb2->set_sb2_auxiliary_compressor(flagOf(panel, "sb2_auxiliary_compressor"));
    sb2->set_sb2_drivers_cab(flagOf(panel, "sb2_drivers_cab"));
    sb2->set_sb2_lighting_machine_room(flagOf(panel, "sb2_lighting_machine_room"));
    sb2->set_sb2_power_supply_gate_units(flagOf(panel, "sb2_power_supply_gate_units"));
    sb2->set_sb2_commissioning_1(flagOf(panel, "sb2_commissioning_1"));
    sb2->set_sb2_vigilance_control(flagOf(panel, "sb2_vigilance_control"));
    sb2->set_sb2_pneumatic_panel(flagOf(panel, "sb2_pneumatic_panel"));
    sb2->set_sb2_commissioning_2(flagOf(panel, "sb2_commissioning_2"));
    sb2->set_sb2_24v_48v_power_supply(flagOf(panel, "sb2_24v_48v_power_supply"));
    sb2->set_sb2_marker_lights(flagOf(panel, "sb2_marker_lights"));
    sb2->set_sb2_lighting_front(flagOf(panel, "sb2_lighting_front"));
    sb2->set_sb2_electronics_traction_converter_2(flagOf(panel, "sb2_electronics_traction_converter_2"));
    sb2->set_sb2_monitoring(flagOf(panel, "sb2_monitoring"));
    sb2->set_sb2_electronics_aux_converter_2(flagOf(panel, "sb2_electronics_aux_converter_2"));
    sb2->set_sb2_electronics_aux_converter_3(flagOf(panel, "sb2_electronics_aux_converter_3"));
    sb2->set_sb2_central_electronics_2(flagOf(panel, "sb2_central_electronics_2"));
    sb2->set_sb2_central_electronics_3(flagOf(panel, "sb2_central_electronics_3"));
    sb2->set_sb2_memotel_speedometer(flagOf(panel, "sb2_memotel_speedometer"));
    sb2->set_sb2_aux_contactor(flagOf(panel, "sb2_aux_contactor"));
    sb2->set_sb2_power_supply_air_dryer(flagOf(panel, "sb2_power_supply_air_dryer"));
    sb2->set_sb2_mcb_for_air_dryer(flagOf(panel, "sb2_mcb_for_air_dryer"));
    sb2->set_sb2_output_battery_charger(flagOf(panel, "sb2_output_battery_charger"));
    sb2->set_sb2_control_circuit_locomotive(flagOf(panel, "sb2_control_circuit_locomotive"));
}

void fillHb2(proto::wag9::fs::Hb2* hb2, QWidget* panel)
{
    hb2->set_hb2_transformer_oil_pump(flagOf(panel, "hb2_transformer_oil_pump"));
    hb2->set_hb2_converter_oil_pump(flagOf(panel, "hb2_converter_oil_pump"));
    hb2->set_hb2_main_compressor(flagOf(panel, "hb2_main_compressor"));
    hb2->set_hb2_traction_motor_blower(flagOf(panel, "hb2_traction_motor_blower"));
    hb2->set_hb2_scavenge_blower_tm(flagOf(panel, "hb2_scavenge_blower_tm"));
    hb2->set_hb2_vent_oil_cooler(flagOf(panel, "hb2_vent_oil_cooler"));
    hb2->set_hb2_m_c_room_blower(flagOf(panel, "hb2_m_c_room_blower"));
    hb2->set_hb2_scavenge_blower_mr(flagOf(panel, "hb2_scavenge_blower_mr"));
    hb2->set_hb2_circuit_breaker_for_cab_ac(flagOf(panel, "hb2_circuit_breaker_for_cab_ac"));
}

QStringList pantographFrames(const QString& direction)
{
    QStringList frames;
    for (int i = 1; i <= kPantographFrameCount; ++i)
    {
        frames << QStringLiteral(":/Wag9/fs/panto/%1/%2.png").arg(direction).arg(i, 2, 10, QLatin1Char('0'));
    }
    return frames;
}

}

Wag9Fs::Wag9Fs(QWidget *parent)
    : QFrame(parent)
{}

void Wag9Fs::init(std::shared_ptr<Wag9Configuration const> config)
{
    m_staticConfig = std::move(config);

    ui.setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::WindowStaysOnTopHint, m_staticConfig->fsSettings().topMost);
    move(m_staticConfig->fsSettings().position);
    prepareZones();
    createPantographs();

    const QStringList zones = { kSb1, kHb1, kSb2, kHb2 };

    m_ccbPanel = new Wag9PneumoPanel(this);
    m_ccbPanel->hide();
    m_popups.insert(kPneumoCcb, m_ccbPanel);

    m_e70Panel = new Wag9PneumoE70Panel(this);
    m_e70Panel->hide();
    m_popups.insert(kPneumoE70, m_e70Panel);

    for (const QString& zone : zones)
    {
        m_popups.insert(zone, new Wag9CubiclePanel(zone, this));
        m_popups[zone]->hide();
    }

    ui.background->lower();
    ui.pneumoCcb->raise();
    ui.pneumoE70->raise();
    ui.hintsButton->raise();

    createServer();
}

void Wag9Fs::prepareZones()
{
    auto* links = new QButtonGroup(this);
    links->setExclusive(false);
    m_linkButtons = findChildren<QPushButton*>(QRegularExpression(QStringLiteral("^link_")));
    for (QPushButton* button : m_linkButtons)
    {
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);
        links->addButton(button);
    }
    connect(links, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked), this, &Wag9Fs::onLinkClicked);

    ui.frameView->installEventFilter(this);

    m_hintLabels = findChildren<QLabel*>(QRegularExpression(QStringLiteral("^hint_")));
    for (QLabel* label : m_hintLabels)
    {
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    connect(ui.hintsButton, &QPushButton::toggled, this, &Wag9Fs::setHintsVisible);

    auto *pneumoModes = new QButtonGroup(this);

    pneumoModes->setExclusive(true);
    pneumoModes->addButton(ui.pneumoCcb);
    pneumoModes->addButton(ui.pneumoE70);

    connect(ui.pneumoCcb, &QPushButton::toggled, this, &Wag9Fs::onPneumoModeToggled);
    connect(ui.pneumoE70, &QPushButton::toggled, this, &Wag9Fs::onPneumoModeToggled);
}

void Wag9Fs::onLinkClicked(QAbstractButton* button)
{
    if (!button->isChecked())
    {
        closeFrame();
        return;
    }

    for (QPushButton* other : m_linkButtons)
    {
        if (other != button)
        {
            other->setChecked(false);
        }
    }

    QString zone = button->property("zone").toString();
    QWidget* panel = zone == kPneumoCcb ? getPneumoPanel() : m_popups.value(zone);
    if (panel)
    {
        showPopup(panel);
    }
}

void Wag9Fs::setHintsVisible(bool enabled)
{
    for (QLabel* label : m_hintLabels)
    {
        label->setVisible(enabled);
        if (enabled)
        {
            label->raise();
        }
    }

    if (ui.frameView->isVisible())
    {
        ui.frameView->raise();
    }

    for (QWidget* popup : m_popups)
    {
        if (popup->isVisible())
        {
            popup->raise();
        }
    }
}

void Wag9Fs::onPneumoModeToggled(bool checked)
{
    if (checked)
    {
        setBrakeSystem(ui.pneumoE70->isChecked() ? BrakeSystem::E70 : BrakeSystem::Ccb);
        ui.background->setPixmap(m_brakeSystem == BrakeSystem::E70 ? kBackgroundE70 : kBackgroundCcb);
    }
}

void Wag9Fs::setBrakeSystem(BrakeSystem system)
{
    const bool e70 = system == BrakeSystem::E70;

    const QSignalBlocker blockCcb(ui.pneumoCcb);
    const QSignalBlocker blockE70(ui.pneumoE70);

    m_brakeSystem = system;

    if (e70)
    {
        ui.pneumoE70->setChecked(true);
    }
    else
    {
        ui.pneumoCcb->setChecked(true);
    }
}

QWidget *Wag9Fs::getPneumoPanel() const
{
    if (m_brakeSystem == BrakeSystem::E70)
    {
        return m_e70Panel;
    }
    return m_ccbPanel;
}

void Wag9Fs::createPantographs()
{
    const QStringList pantograph1 = pantographFrames(QStringLiteral("left"));
    const QStringList pantograph2 = pantographFrames(QStringLiteral("right"));

    m_pantograph1[0] = addPantographView(pantograph1, ui.pan_right_1);
    m_pantograph1[1] = addPantographView(pantograph1, ui.pan_left_1);

    connect(ui.pan_right_1, &QPushButton::clicked, this, [this]
    {
        togglePantographPair(m_pantograph1[0], m_pantograph1[1]);
    });
    connect(ui.pan_left_1, &QPushButton::clicked, this, [this]
    {
        togglePantographPair(m_pantograph1[0], m_pantograph1[1]);
    });

    m_pantograph2[0] = addPantographView(pantograph2, ui.pan_right_2);
    m_pantograph2[1] = addPantographView(pantograph2, ui.pan_left_2);

    connect(ui.pan_right_2, &QPushButton::clicked, this, [this]
    {
        togglePantographPair(m_pantograph2[0], m_pantograph2[1]);
    });
    connect(ui.pan_left_2, &QPushButton::clicked, this, [this]
    {
        togglePantographPair(m_pantograph2[0], m_pantograph2[1]);
    });
}

KeySwitchButton* Wag9Fs::addPantographView(const QStringList& frames, QPushButton* clickArea)
{
    QStringList framesDown = frames;
    std::reverse(framesDown.begin(), framesDown.end());

    auto view = new KeySwitchButton(this);
    view->setAnimationOnList(frames);
    view->setAnimationOffList(framesDown);
    view->setGeometry(clickArea->geometry());
    view->setControlButtonGeometry(view->rect());
    view->setSilent(true);
    view->init();
    view->setClickable(false);
    view->show();

    clickArea->setFocusPolicy(Qt::NoFocus);
    clickArea->setCursor(Qt::PointingHandCursor);
    clickArea->raise();
    return view;
}

void Wag9Fs::togglePantographPair(KeySwitchButton* a, KeySwitchButton* b)
{
    if (a->animationState() != KeySwitchButton::AnimationState::None
        || b->animationState() != KeySwitchButton::AnimationState::None)
    {
        return;
    }

    const auto next = a->state() == ControlButtonState::On
        ? ControlButtonState::Off
        : ControlButtonState::On;
    a->setState(next);
    b->setState(next);
}

void Wag9Fs::showPopup(QWidget* panel)
{
    hidePopups();

    m_frameRect = QRect(
        (width() - panel->width()) / 2,
        (height() - panel->height()) / 2,
        panel->width(),
        panel->height());

    ui.frameView->show();
    ui.frameView->raise();
    panel->setGeometry(m_frameRect);
    panel->show();
    panel->raise();
}

void Wag9Fs::hidePopups()
{
    for (QWidget* popup : m_popups)
    {
        popup->hide();
    }
}

void Wag9Fs::closeFrame()
{
    for (QPushButton* button : m_linkButtons)
    {
        button->setChecked(false);
    }

    hidePopups();
    ui.frameView->hide();
}

bool Wag9Fs::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui.frameView && event->type() == QEvent::MouseButtonPress)
    {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (!m_frameRect.contains(mouseEvent->pos()))
        {
            closeFrame();
            return true;
        }
    }

    return QFrame::eventFilter(watched, event);
}

void Wag9Fs::createServer()
{
    try
    {
        m_server = std::make_unique<Wag9FsServer>(m_staticConfig->fsSettings().bindAddress);
    }
    catch (const std::exception& ex)
    {
        qCritical() << "Failed to create server" << ex.what();
        return;
    }

    connect(m_server.get(), &Wag9FsServer::initMessageReceived, this, &Wag9Fs::handleInitMessage);
    connect(m_server.get(), &Wag9FsServer::messageReceived, this, &Wag9Fs::handleIncomingPackageFromSim);

    m_reportTimer.setInterval(kReportIntervalMs);
    connect(&m_reportTimer, &QTimer::timeout, this, &Wag9Fs::reportStationState);
    m_reportTimer.start();
}

void Wag9Fs::reportStationState()
{
    if (!m_server || !m_server->hasClient())
    {
        return;
    }

    proto::wag9::fs::FaultStationOutgoingMessage msg;
    if (m_brakeSystem == BrakeSystem::E70)
    {
        fillPneumoE70(msg.mutable_pneumo_e70(), m_e70Panel);
    }
    else
    {
        fillPneumoCcb(msg.mutable_pneumo_ccb(), m_ccbPanel);
    }

    fillSb1(msg.mutable_sb1(), m_popups.value(kSb1));
    fillHb1(msg.mutable_hb1(), m_popups.value(kHb1));
    fillSb2(msg.mutable_sb2(), m_popups.value(kSb2));
    fillHb2(msg.mutable_hb2(), m_popups.value(kHb2));

    msg.set_pantograph_1_raised(m_pantograph1[0]->state() == ControlButtonState::On);
    msg.set_pantograph_2_raised(m_pantograph2[0]->state() == ControlButtonState::On);

    m_server->sendMessage(msg);
}

void Wag9Fs::handleInitMessage(const proto::FaultStationInitMessage& msg)
{
    qDebug() << "Init from sim, vehicles:" << msg.vehicle_numbers_size();
}

void Wag9Fs::handleIncomingPackageFromSim(const proto::wag9::fs::FaultStationIncomingMessage& msg)
{
    if (msg.has_ccb_manometer_pressure())
    {
        m_ccbPanel->setPressure(msg.ccb_manometer_pressure());
    }

    if (msg.has_e70_manometer_pressure())
    {
        m_e70Panel->setPressure(msg.e70_manometer_pressure());
    }

    if (msg.has_e70_processor_fault())
    {
        m_e70Panel->setLed(QStringLiteral("processor_fault"), msg.e70_processor_fault());
    }

    if (msg.has_e70_system_fault())
    {
        m_e70Panel->setLed(QStringLiteral("system_fault"), msg.e70_system_fault());
    }

    if (msg.has_e70_psu_normal())
    {
        m_e70Panel->setLed(QStringLiteral("psu_normal"), msg.e70_psu_normal());
    }
}