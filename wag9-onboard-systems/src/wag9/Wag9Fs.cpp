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
    e70->set_processor_fault(0);
    e70->set_system_fault(0);
    e70->set_psu_normal(3);
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
    for (int i = 1; i <= 10; ++i)
    {
        frames << QStringLiteral(":/Wag9/fs/panto/%1/%2.png").arg(direction).arg(i, 2, 10, QLatin1Char('0'));
    }
    return frames;
}

}

Wag9Fs::Wag9Fs(QWidget *parent)
    : QFrame(parent)
    , m_panelFactory(std::make_unique<Wag9::PanelFactory>())
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

    const QStringList zones = {
        kSb1,
        kHb1,
        kSb2,
        kHb2
    };

    m_popups.insert(kPneumoCcb, new Wag9PneumoPanel(this));
    m_popups[kPneumoCcb]->hide();

    m_popups.insert(kPneumoE70, new Wag9PneumoE70Panel(this));
    m_popups[kPneumoE70]->hide();

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
    m_linkButtons = findChildren<QPushButton*>(QRegularExpression(QStringLiteral("^link_")));
    for (QPushButton* button : m_linkButtons)
    {
        button->setFocusPolicy(Qt::NoFocus);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this, [this, button](bool checked)
        {
            if (!checked)
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
            if (zone == kPneumoCcb && ui.pneumoE70->isChecked())
            {
                zone = kPneumoE70;
            }

            QWidget* panel = m_popups.value(zone);
            if (panel)
            {
                showPopup(panel);
            }
        });
    }

    ui.frameView->installEventFilter(this);

    const auto hintLabels = findChildren<QLabel*>(QRegularExpression(QStringLiteral("^hint_")));
    for (QLabel* label : hintLabels)
    {
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

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
    });

    auto *pneumoModes = new QButtonGroup(this);
    pneumoModes->setExclusive(true);
    pneumoModes->addButton(ui.pneumoCcb);
    pneumoModes->addButton(ui.pneumoE70);

    const auto applyPneumoBackground = [this]()
    {
        const bool e70 = ui.pneumoE70->isChecked();
        ui.background->setPixmap(e70
            ? QStringLiteral(":/Wag9/fs/WAG9_FS_Background_E70.png")
            : QStringLiteral(":/Wag9/fs/WAG9_FS_Background_CCB.png"));
    };

    const auto onModeToggled = [applyPneumoBackground](bool checked)
    {
        if (checked)
        {
            applyPneumoBackground();
        }
    };

    connect(ui.pneumoCcb, &QPushButton::toggled, this, onModeToggled);
    connect(ui.pneumoE70, &QPushButton::toggled, this, onModeToggled);
}

void Wag9Fs::createPantographs()
{
    const auto addView = [this](const QStringList& frames, QPushButton* clickArea) -> KeySwitchButton*
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
    };

    const auto bindPair = [this](QPushButton* firstClick, QPushButton* secondClick,
        KeySwitchButton* a, KeySwitchButton* b)
    {
        const auto toggle = [a, b]()
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
        };

        connect(firstClick, &QPushButton::clicked, this, toggle);
        connect(secondClick, &QPushButton::clicked, this, toggle);
    };

    const QStringList pantograph1 = pantographFrames(QStringLiteral("left"));
    const QStringList pantograph2 = pantographFrames(QStringLiteral("right"));

    m_pantograph1[0] = addView(pantograph1, ui.pan_right_1);
    m_pantograph1[1] = addView(pantograph1, ui.pan_left_1);
    bindPair(ui.pan_right_1, ui.pan_left_1, m_pantograph1[0], m_pantograph1[1]);

    m_pantograph2[0] = addView(pantograph2, ui.pan_right_2);
    m_pantograph2[1] = addView(pantograph2, ui.pan_left_2);
    bindPair(ui.pan_right_2, ui.pan_left_2, m_pantograph2[0], m_pantograph2[1]);
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

    m_reportTimer.setInterval(100);
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
    if (ui.pneumoE70->isChecked())
    {
        fillPneumoE70(msg.mutable_pneumo_e70(), m_popups.value(kPneumoE70));
    }
    else
    {
        fillPneumoCcb(msg.mutable_pneumo_ccb(), m_popups.value(kPneumoCcb));
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

void Wag9Fs::handleIncomingPackageFromSim(const proto::wag9::fs::FaultStationIncomingMessage&)
{
}