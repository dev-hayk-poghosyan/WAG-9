#include "Wag9PneumoE70Panel.h"
#include "Wag9SpriteSwitch.h"

#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringList>

Wag9PneumoE70Panel::Wag9PneumoE70Panel(QWidget* parent)
    : QWidget(parent)
{
    const QPixmap framePix(QStringLiteral(":/Wag9/fs/pneumo/e70/BaseFrame.png"));
    setFixedSize(framePix.size());

    auto* frame = new QLabel(this);
    frame->setPixmap(framePix);
    frame->setGeometry(0, 0, framePix.width(), framePix.height());
    frame->setAttribute(Qt::WA_TransparentForMouseEvents);

    const QPixmap picturePix(QStringLiteral(":/Wag9/fs/pneumo/e70/Frame.png"));
    auto* picture = new QLabel;
    picture->setPixmap(picturePix);
    picture->setFixedSize(picturePix.size());

    // Light opening inside BaseFrame.png, under the green title.
    auto* scroll = new QScrollArea(this);
    scroll->setGeometry(6, 57, 988, 937);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(false);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    const int viewWidth = scroll->width() - scroll->verticalScrollBar()->sizeHint().width();
    const QPixmap fitted = picturePix.scaledToWidth(viewWidth, Qt::SmoothTransformation);
    picture->setPixmap(fitted);
    picture->setFixedSize(fitted.size());

    const qreal scale = qreal(viewWidth) / picturePix.width();
    const QPixmap ledOff(QStringLiteral(":/Wag9/fs/pneumo/e70/LED_Off.png"));
    const auto placeLed = [picture, ledOff, scale](const char* name, int x, int y)
    {
        auto* led = new QLabel(picture);
        led->setObjectName(QString::fromLatin1(name));
        led->setPixmap(ledOff);
        led->setFixedSize(ledOff.size());
        led->move(qRound(x * scale) - ledOff.width() / 2, qRound(y * scale) - ledOff.height() / 2);
        led->setAttribute(Qt::WA_TransparentForMouseEvents);
    };
    // Centres of the dark lamps on Frame.png: PROCESSOR FAULT, SYSTEM FAULT, PSU NORMAL.
    placeLed("processor_fault", 577, 26);
    placeLed("system_fault", 577, 66);
    placeLed("psu_normal", 577, 107);

    // pivotX/pivotY are on Frame.png. spritePivot is the hinge inside the unscaled frame;
    // (-1, -1) means the middle of the picture.
    const auto place = [picture, scale](const char* name, const QString& sprite, const QList<int>& indexes,
        int initial, int pivotX, int pivotY, int spritePivotX, int spritePivotY)
    {
        const QStringList frames = framesOf(QStringLiteral("pneumo/e70"), sprite, indexes);
        const QPixmap sample(frames.at(0));
        const qreal originX = spritePivotX < 0 ? sample.width() / 2.0 : spritePivotX;
        const qreal originY = spritePivotY < 0 ? sample.height() / 2.0 : spritePivotY;
        const QPoint center(
            qRound((pivotX - (originX - sample.width() / 2.0)) * scale),
            qRound((pivotY - (originY - sample.height() / 2.0)) * scale));
        auto* sw = new Wag9SpriteSwitch(frames, initial, center, picture, SwitchActuation::Cycle, scale);
        sw->setObjectName(QString::fromLatin1(name));
    };

    // Centre of the white AUTO / I / II plate.
    place("pan_sel_sw", QStringLiteral("SwitchBlack10"), {1, 2, 3}, 0, 85, 390, -1, -1);
    // Hinge of SwitchBlack11 is the round end of the arm, not the picture centre.
    place("switch_1", QStringLiteral("SwitchBlack11"), {1, 2}, 0, 670, 757, 176, 80);
    place("switch_2", QStringLiteral("SwitchBlack11"), {1, 2}, 0, 904, 757, 176, 80);
    place("pan2", QStringLiteral("SwitchRed2"), {1, 2}, 0, 699, 996, -1, -1);
    place("pan1", QStringLiteral("SwitchRed2"), {1, 2}, 0, 934, 997, -1, -1);
    place("vcb", QStringLiteral("SwitchRed2"), {1, 2}, 0, 934, 1093, -1, -1);
    // Round seat to the right of the gauge, above the PANTOG plate.
    place("pantog", QStringLiteral("SwitchKey"), {1, 2}, 0, 801, 1090, -1, -1);
    place("dead_engine", QStringLiteral("SwitchRed2"), {1, 2}, 0, 377, 1658, -1, -1);
    // Small cock above the BRAKE FEED PIPE plate, not the green valve under it.
    place("brake_feed_pipe", QStringLiteral("SwitchBlack12"), {1, 2}, 0, 881, 1548, 124, 24);
    place("sand_1", QStringLiteral("SwitchRed2"), {1, 2}, 0, 140, 1860, -1, -1);
    place("sand_2", QStringLiteral("SwitchRed2"), {1, 2}, 0, 280, 1860, -1, -1);

    const QPixmap needlePix(QStringLiteral(":/Wag9/fs/pneumo/e70/ArrowBlack1.png"));
    const QPixmap needleFitted = needlePix.scaled(
        qRound(needlePix.width() * scale), qRound(needlePix.height() * scale),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    auto* needle = new QLabel(picture);
    needle->setObjectName(QStringLiteral("manometer"));
    needle->setPixmap(needleFitted);
    needle->setFixedSize(needleFitted.size());
    needle->move(qRound(688 * scale) - needleFitted.width() / 2, qRound(1101 * scale) - needleFitted.height() / 2);
    needle->setAttribute(Qt::WA_TransparentForMouseEvents);

    scroll->setWidget(picture);
}
