#include "Wag9PneumoPanel.h"

#include "Wag9Needle.h"
#include "Wag9SpriteSwitch.h"

#include <QLabel>
#include <QPixmap>
#include <QStringList>

Wag9PneumoPanel::Wag9PneumoPanel(QWidget* parent)
    : QWidget(parent)
{
    const QPixmap backgroundPix(QStringLiteral(":/Wag9/fs/pneumo/ccb/Background.png"));
    setFixedSize(backgroundPix.size());

    auto* background = new QLabel(this);
    background->setPixmap(backgroundPix);
    background->setGeometry(0, 0, backgroundPix.width(), backgroundPix.height());

    m_needle = new Wag9Needle(QPixmap(QStringLiteral(":/Wag9/fs/pneumo/ccb/ArrowBlack1.png")),
        QPoint(826, 294), kPantoGaugeMax, kPantoGaugeSweep, this);

    place("ec_coc", "SwitchBlack2", {47, 82});
    place("tc2_coc", "SwitchBlack2", {47, 147});
    place("tc1_coc", "SwitchBlack2", {47, 211});

    place("sifa_coc", "SwitchRed1", {383, 126});
    place("fp_coc", "SwitchYellow1", {364, 236});

    place("sand1_coc", "SwitchBlack3", {551, 133});
    place("sand2_coc", "SwitchBlack3", {617, 133});

    place("ul_coc", "SwitchBlack2", {683, 136}, 2, 1);
    place("cb_coc", "SwitchBlack2", {745, 136}, 2, 1);

    place("pans_coc", "SwitchKey", {818, 149}, 2, 1);
    place("pan1_coc", "SwitchBlack2", {869, 136});
    place("pan2_coc", "SwitchBlack2", {931, 136});

    place("pan_sel_sw", "SwitchBlack1", {860, 82}, 3);

    place("pass_goods", "SwitchBlack4", {241, 612});
    place("per_coc", "SwitchRed1", {681, 677});
    place("dead_engine", "SwitchYellow2", {831, 696});
    place("gp", "SwitchYellow3", {204, 878});
}

void Wag9PneumoPanel::setPressure(double value)
{
    m_needle->setValue(value);
}

void Wag9PneumoPanel::place(const char* name, const QString& sprite, const QPoint& topLeft,
    int frameCount, int initial)
{
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), sprite, frameCount), initial,
        topLeft.x(), topLeft.y(), QString::fromLatin1(name));
}