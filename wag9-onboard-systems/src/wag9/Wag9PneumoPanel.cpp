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

    auto* needle = new Wag9Needle(QPixmap(QStringLiteral(":/Wag9/fs/pneumo/ccb/ArrowBlack1.png")),
            QPoint(826, 294), 10, 270, this);
    needle->setObjectName(QStringLiteral("manometer"));

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 47, 82, "ec_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 47, 147, "tc2_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 47, 211, "tc1_coc");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchRed1"), {1, 2}), 0, 383, 126, "sifa_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchYellow1"), {1, 2}), 0, 364, 236, "fp_coc");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack3"), {1, 2}), 0, 551, 133, "sand1_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack3"), {1, 2}), 0, 617, 133, "sand2_coc");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 1, 683, 136, "ul_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 1, 745, 136, "cb_coc");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchKey"), {1, 2}), 0, 818, 149, "pans_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 869, 136, "pan1_coc");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 931, 136, "pan2_coc");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack1"), {1, 2, 3}), 0, 860, 82, "pan_sel_sw");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack4"), {1, 2}), 0, 241, 612, "pass_goods");
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchRed1"), {1, 2}), 0, 681, 677, "per_coc");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchYellow2"), {1, 2}), 0, 831, 696, "dead_engine");

    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchYellow3"), {1, 2}), 0, 204, 878, "gp");
}