#include "Wag9PneumoE70Panel.h"

#include "Wag9Needle.h"
#include "Wag9SpriteSwitch.h"

#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QStringList>

namespace
{

QString ledPicture(proto::wag9::fs::LedState state)
{
    switch (state)
    {
        case proto::wag9::fs::LED_STATE_RED:
            return QStringLiteral(":/Wag9/fs/pneumo/e70/LED_Red_On.png");
        case proto::wag9::fs::LED_STATE_YELLOW:
            return QStringLiteral(":/Wag9/fs/pneumo/e70/LED_Yellow_On.png");
        case proto::wag9::fs::LED_STATE_GREEN:
            return QStringLiteral(":/Wag9/fs/pneumo/e70/LED_Green_On.png");
        case proto::wag9::fs::LED_STATE_WHITE:
            return QStringLiteral(":/Wag9/fs/pneumo/e70/LED_White_On.png");
        default:
            return QStringLiteral(":/Wag9/fs/pneumo/e70/LED_Off.png");
    }
}

}

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
    m_picture = picture;
    m_scale = scale;


    placeLed("processor_fault", {577, 26}, proto::wag9::fs::LED_STATE_OFF);
    placeLed("system_fault", {577, 66}, proto::wag9::fs::LED_STATE_OFF);
    placeLed("psu_normal", {577, 107}, proto::wag9::fs::LED_STATE_GREEN);

    place("pan_sel_sw", QStringLiteral("SwitchBlack10"), {85, 390}, 3);

    place("switch_1", QStringLiteral("SwitchBlack11"), {670, 757}, 2, 0, QPoint(176, 80));
    place("switch_2", QStringLiteral("SwitchBlack11"), {904, 757}, 2, 0, QPoint(176, 80));
    place("pan2", QStringLiteral("SwitchRed2"), {699, 996});
    place("pan1", QStringLiteral("SwitchRed2"), {934, 997});
    place("vcb", QStringLiteral("SwitchRed2"), {934, 1093});

    place("pantog", QStringLiteral("SwitchKey"), {801, 1090});
    place("dead_engine", QStringLiteral("SwitchRed2"), {372, 1658}, 2, 1);

    place("brake_feed_pipe", QStringLiteral("SwitchBlack12"), {881, 1548}, 2, 0, QPoint(124, 24));
    place("sand_1", QStringLiteral("SwitchRed2"), {125, 1860});
    place("sand_2", QStringLiteral("SwitchRed2"), {280, 1860});

    const QPixmap needlePix(QStringLiteral(":/Wag9/fs/pneumo/e70/ArrowBlack1.png"));
    const QPixmap needleFitted = needlePix.scaled(
        qRound(needlePix.width() * scale), qRound(needlePix.height() * scale),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    m_needle = new Wag9Needle(needleFitted, QPoint(qRound(688 * scale), qRound(1101 * scale)),
            kPantoGaugeMax, kPantoGaugeSweep, picture);

    scroll->setWidget(picture);
}

void Wag9PneumoE70Panel::setPressure(double value)
{
    m_needle->setValue(value);
}

void Wag9PneumoE70Panel::setLed(const QString &name, proto::wag9::fs::LedState state)
{
    auto* led = findChild<QLabel*>(name);
    if (!led)
    {
        qWarning() << "Missing led" << name;
        return;
    }
    led->setPixmap(QPixmap(ledPicture(state)));
}

void Wag9PneumoE70Panel::place(const char* name, const QString& sprite, const QPoint& pivot,
    int frameCount, int initial, std::optional<QPoint> hinge)
{
    const QStringList frames = framesOf(QStringLiteral("pneumo/e70"), sprite, frameCount);
    const QPixmap sample(frames.at(0));
    const QPointF middle(sample.width() / 2.0, sample.height() / 2.0);
    const QPointF offset = hinge ? QPointF(*hinge) - middle : QPointF();
    const QPoint center = ((QPointF(pivot) - offset) * m_scale).toPoint();

    auto* sw = new Wag9SpriteSwitch(frames, initial, center, m_picture, SwitchActuation::Cycle, m_scale);
    sw->setObjectName(QString::fromLatin1(name));
}

void Wag9PneumoE70Panel::placeLed(const char* name, const QPoint& pos, proto::wag9::fs::LedState state)
{
    const QPixmap pix(ledPicture(state));
    auto* led = new QLabel(m_picture);
    led->setObjectName(QString::fromLatin1(name));
    led->setPixmap(pix);
    led->setFixedSize(pix.size());
    led->move(qRound(pos.x() * m_scale) - pix.width() / 2, qRound(pos.y() * m_scale) - pix.height() / 2);
    led->setAttribute(Qt::WA_TransparentForMouseEvents);
}