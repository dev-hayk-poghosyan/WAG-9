#include "Wag9PneumoPanel.h"
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

    auto* needle = new QLabel(this);
    const QPixmap needlePix(QStringLiteral(":/Wag9/fs/pneumo/ccb/ArrowBlack1.png"));
    needle->setPixmap(needlePix);
    needle->setFixedSize(needlePix.size());
    needle->move(770, 238);
    needle->setAttribute(Qt::WA_TransparentForMouseEvents);

    // EC-COC, TC2-COC, TC1-COC.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 47, 82);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 47, 147);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 0, 47, 211);

    // 21(SIFA)-COC, FP-COC.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchRed1"), {1, 2}), 0, 383, 126);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchYellow1"), {1, 2}), 0, 364, 236);

    // SAND1-COC, SAND2-COC.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack3"), {1, 2}), 0, 551, 133);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack3"), {1, 2}), 0, 617, 133);

    // UL-COC, CB-COC. The second frame is the resting position.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 1, 683, 136);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 1, 745, 136);

    // PANS COC, PAN1.COC, PAN2.COC.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchKey"), {1, 2}), 0, 818, 149);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 1, 869, 136);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack2"), {1, 2}), 1, 931, 136);

    // PAN-SEL-SW has three positions.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack1"), {1, 2, 3}), 0, 860, 82);

    // PASS/GOODS, PER-COC.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchBlack4"), {1, 2}), 0, 241, 612);
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchRed1"), {1, 2}), 0, 681, 677);

    // DEAD ENGINE. The second frame is the resting position.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchYellow2"), {1, 2}), 1, 831, 696);

    // G/P.
    addSwitch(this, framesOf(QStringLiteral("pneumo/ccb"), QStringLiteral("SwitchYellow3"), {1, 2}), 0, 204, 878);
}