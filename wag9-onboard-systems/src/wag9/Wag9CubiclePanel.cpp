#include "Wag9CubiclePanel.h"
#include "Wag9SpriteSwitch.h"

#include <QLabel>
#include <QPixmap>
#include <QStringList>

namespace
{

void addPair(QWidget* parent, const QString& sprite, int x, int y, const QString& name)
{
    addSwitch(parent, framesOf(QStringLiteral("cubicle"), sprite, 2), 0, x, y, name);
}

void addRow(QWidget* parent, const QString& sprite, int y, const QList<int>& xs, const QStringList& names)
{
    Q_ASSERT(xs.size() == names.size());
    for (int i = 0; i < xs.size(); ++i)
    {
        addPair(parent, sprite, xs.at(i), y, names.at(i));
    }
}

void buildSb1(QWidget* parent)
{
    const QStringList twoWay = framesOf(QStringLiteral("cubicle"), QStringLiteral("SwitchBlack8"), 2);
    const QStringList fourWay = framesOf(QStringLiteral("cubicle"), QStringLiteral("SwitchBlack8"), 4);
    const QStringList blueFrames = framesOf(QStringLiteral("cubicle"), QStringLiteral("SwitchBlue"), 2);

    addSwitch(parent, twoWay, 0, 65, 110, "sb1_failure_mode_operation");
    addSwitch(parent, fourWay, 0, 211, 110, "sb1_bogie_cut_out");
    addSwitch(parent, twoWay, 0, 357, 110, "sb1_configuration_switch");
    addSwitch(parent, twoWay, 0, 502, 110, "sb1_vigilance_device_cut_off");
    addSwitch(parent, blueFrames, 0, 729, 132, "sb1_simulation");

    addSwitch(parent,
        framesOf(QStringLiteral("cubicle"), QStringLiteral("BTN_YellowLamp1"), 4),
        0, 853, 124, "sb1_configuration_button", SwitchActuation::Lamp);

    addRow(parent, QStringLiteral("SwitchBlack6"), 342,
        {47, 90, 133, 176, 262, 305, 348, 391, 435, 478, 521},
        {QStringLiteral("sb1_drivers_cab"), QStringLiteral("sb1_panto_vcb_control"),
         QStringLiteral("sb1_24v_48v_power_supply"), QStringLiteral("sb1_lighting_front"),
         QStringLiteral("sb1_electronics_traction_converter_1"), QStringLiteral("sb1_power_supply_gate_units"),
         QStringLiteral("sb1_monitoring"), QStringLiteral("sb1_electronics_aux_converter_1"),
         QStringLiteral("sb1_central_electronics_1"), QStringLiteral("sb1_central_electronics_2"),
         QStringLiteral("sb1_speed_sensor")});
}

void buildHb1(QWidget* parent)
{
    addPair(parent, QStringLiteral("SwitchBlack5"), 47, 124, QStringLiteral("hb1_transformer_oil_pump"));
    addPair(parent, QStringLiteral("SwitchBlack7"), 236, 118, QStringLiteral("hb1_converter_oil_pump"));

    addRow(parent, QStringLiteral("SwitchBlack5"), 124, {338, 530, 676, 820},
        {QStringLiteral("hb1_main_compressor"), QStringLiteral("hb1_traction_motor_blower"),
         QStringLiteral("hb1_scavenge_blower_tm"), QStringLiteral("hb1_vent_oil_cooler")});

    addRow(parent, QStringLiteral("SwitchBlack5"), 343, {47, 193},
        {QStringLiteral("hb1_mrb1"), QStringLiteral("hb1_scavenge_blower_mr")});

    addRow(parent, QStringLiteral("SwitchBlack6"), 343, {573, 718, 863},
        {QStringLiteral("hb1_cab_ventilation"), QStringLiteral("hb1_cab_heater"),
         QStringLiteral("hb1_crew_fan")});
}

void buildHb2(QWidget* parent)
{
    addPair(parent, QStringLiteral("SwitchBlack5"), 47, 124, QStringLiteral("hb2_transformer_oil_pump"));
    addPair(parent, QStringLiteral("SwitchBlack7"), 236, 118, QStringLiteral("hb2_converter_oil_pump"));

    addRow(parent, QStringLiteral("SwitchBlack5"), 124, {338, 530, 676, 820},
        {QStringLiteral("hb2_main_compressor"), QStringLiteral("hb2_traction_motor_blower"),
         QStringLiteral("hb2_scavenge_blower_tm"), QStringLiteral("hb2_vent_oil_cooler")});

    addRow(parent, QStringLiteral("SwitchBlack5"), 343, {47, 193, 338},
        {QStringLiteral("hb2_m_c_room_blower"), QStringLiteral("hb2_scavenge_blower_mr"),
         QStringLiteral("hb2_circuit_breaker_for_cab_ac")});
}

void buildSb2(QWidget* parent)
{
    addRow(parent, QStringLiteral("SwitchBlack6"), 124, {90, 235, 380, 526},
        {QStringLiteral("sb2_auxiliary_compressor"), QStringLiteral("sb2_drivers_cab"),
         QStringLiteral("sb2_lighting_machine_room"), QStringLiteral("sb2_power_supply_gate_units")});

    addRow(parent, QStringLiteral("SwitchBlack6"), 342,
        {101, 144, 187, 230, 273, 316, 359, 446, 490, 533, 576, 619, 662, 705, 748, 791},
        {QStringLiteral("sb2_commissioning_1"), QStringLiteral("sb2_vigilance_control"),
         QStringLiteral("sb2_pneumatic_panel"), QStringLiteral("sb2_commissioning_2"),
         QStringLiteral("sb2_24v_48v_power_supply"), QStringLiteral("sb2_marker_lights"),
         QStringLiteral("sb2_lighting_front"), QStringLiteral("sb2_electronics_traction_converter_2"),
         QStringLiteral("sb2_monitoring"), QStringLiteral("sb2_electronics_aux_converter_2"),
         QStringLiteral("sb2_electronics_aux_converter_3"), QStringLiteral("sb2_central_electronics_2"),
         QStringLiteral("sb2_central_electronics_3"), QStringLiteral("sb2_memotel_speedometer"),
         QStringLiteral("sb2_aux_contactor"), QStringLiteral("sb2_power_supply_air_dryer")});

    addPair(parent, QStringLiteral("SwitchBlack9"), 840, 343, QStringLiteral("sb2_mcb_for_air_dryer"));

    const QStringList breaker = {
        QStringLiteral(":/Wag9/fs/cubicle/SwitchWhite1_01.png"),
        QStringLiteral(":/Wag9/fs/cubicle/SwitchWhite1_02.png")
    };

    addSwitch(parent, breaker, 0, 101, 669, "sb2_output_battery_charger");
    addSwitch(parent, breaker, 0, 316, 669, "sb2_control_circuit_locomotive");
}

}

Wag9CubiclePanel::Wag9CubiclePanel(const QString& panel, QWidget* parent)
    : QWidget(parent)
{
    const QPixmap background(QStringLiteral(":/Wag9/fs/cubicle/%1_Background.png").arg(panel));
    setFixedSize(background.size());

    auto* plate = new QLabel(this);
    plate->setPixmap(background);
    plate->setGeometry(0, 0, background.width(), background.height());

    if (panel == QLatin1String("SB1"))
    {
        buildSb1(this);
    }
    else if (panel == QLatin1String("HB1"))
    {
        buildHb1(this);
    }
    else if (panel == QLatin1String("HB2"))
    {
        buildHb2(this);
    }
    else if (panel == QLatin1String("SB2"))
    {
        buildSb2(this);
    }
}
