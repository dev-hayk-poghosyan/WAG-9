#include "Wag9CubiclePanel.h"
#include "Wag9SpriteSwitch.h"


#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QStringList>

namespace
{

void addPair(QWidget* parent, const QString& sprite, int x, int y)
{
    addSwitch(parent, framesOf(QStringLiteral("cubicle"), sprite, {1, 2}), 0, x, y);
}

void addRow(QWidget* parent, const QString& sprite, int y, const QList<int>& xs)
{
    for (int x : xs)
    {
        addPair(parent, sprite, x, y);
    }
}

void buildSb1(QWidget* parent)
{
    const QStringList twoWay = framesOf(QStringLiteral("cubicle"), QStringLiteral("SwitchBlack8"), {1, 2});
    const QStringList fourWay = framesOf(QStringLiteral("cubicle"), QStringLiteral("SwitchBlack8"), {1, 2, 3, 4});

    addSwitch(parent, twoWay, 0, 65, 110);
    addSwitch(parent, fourWay, 0, 211, 110);
    addSwitch(parent, twoWay, 0, 357, 110);
    addSwitch(parent, twoWay, 0, 502, 110);

    addSwitch(parent, {
        QStringLiteral(":/Wag9/fs/cubicle/SwitchBlue_01.png"),
        QStringLiteral(":/Wag9/fs/cubicle/SwitchBlue_02.png")
    }, 0, 729, 132);
    // 01 off, 02 pressed off, 03 pressed on, 04 on.
    addSwitch(parent,
        framesOf(QStringLiteral("cubicle"), QStringLiteral("BTN_YellowLamp1"), {1, 2, 3, 4}),
        0, 853, 124, SwitchActuation::Lamp);

    addRow(parent, QStringLiteral("SwitchBlack6"), 342,
        {47, 90, 133, 176, 262, 305, 348, 391, 435, 478, 521});
}

void buildHb1(QWidget* parent)
{
    addRow(parent, QStringLiteral("SwitchBlack5"), 124, {47, 338, 530, 676, 820});
    addPair(parent, QStringLiteral("SwitchBlack7"), 236, 118);
    addRow(parent, QStringLiteral("SwitchBlack5"), 343, {47, 193});
    addRow(parent, QStringLiteral("SwitchBlack6"), 343, {573, 718, 863});
}

void buildHb2(QWidget* parent)
{
    addRow(parent, QStringLiteral("SwitchBlack5"), 124, {47, 338, 530, 676, 820});
    addPair(parent, QStringLiteral("SwitchBlack7"), 236, 118);
    addRow(parent, QStringLiteral("SwitchBlack5"), 343, {47, 193, 338});
}

void buildSb2(QWidget* parent)
{
    addRow(parent, QStringLiteral("SwitchBlack6"), 124, {90, 235, 380, 526});
    addRow(parent, QStringLiteral("SwitchBlack6"), 342,
        {101, 144, 187, 230, 273, 316, 359, 446, 490, 533, 576, 619, 662, 705, 748, 791});
    addPair(parent, QStringLiteral("SwitchBlack9"), 840, 343);

    const QStringList breaker = {
        QStringLiteral(":/Wag9/fs/cubicle/SwitchWhite1_01.png"),
        QStringLiteral(":/Wag9/fs/cubicle/SwitchWhite1_02.png")
    };
    addSwitch(parent, breaker, 0, 101, 669);
    addSwitch(parent, breaker, 0, 316, 669);
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
