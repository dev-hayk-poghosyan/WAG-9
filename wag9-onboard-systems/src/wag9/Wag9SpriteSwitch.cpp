#include "Wag9SpriteSwitch.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QStringList>

Wag9SpriteSwitch::Wag9SpriteSwitch(const QStringList& frames, int initial, const QPoint& center,
    QWidget* parent, SwitchActuation actuation, qreal scale)
    : QLabel(parent)
    , m_index(initial)
    , m_center(center)
    , m_actuation(actuation)
{
    m_frames.reserve(frames.size());
    for (const QString& frame : frames)
    {
        QPixmap pix(frame);
        if (scale != 1)
        {
            pix = pix.scaled(qRound(pix.width() * scale), qRound(pix.height() * scale),
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        m_frames << pix;
    }
    setCursor(Qt::PointingHandCursor);
    applyFrame();
}

void Wag9SpriteSwitch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        return;
    }

    if (m_actuation == SwitchActuation::Lamp)
    {
        m_index = m_on ? 2 : 1;
    }
    else
    {
        m_index = (m_index + 1) % m_frames.size();
    }
    applyFrame();
    event->accept();
}

void Wag9SpriteSwitch::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_actuation != SwitchActuation::Lamp)
    {
        return;
    }

    if (rect().contains(event->pos()))
    {
        m_on = !m_on;
    }
    m_index = m_on ? 3 : 0;
    applyFrame();
    event->accept();
}

void Wag9SpriteSwitch::applyFrame()
{
    const QPixmap& pix = m_frames.at(m_index);
    setPixmap(pix);
    setFixedSize(pix.size());
    move(m_center.x() - pix.width() / 2, m_center.y() - pix.height() / 2);
}

QStringList framesOf(const QString& folder, const QString& sprite, const QList<int>& indexes)
{
    QStringList frames;
    for (int index : indexes)
    {
        frames << QStringLiteral(":/Wag9/fs/%1/%2_%3.png")
            .arg(folder, sprite)
            .arg(index, 2, 10, QLatin1Char('0'));
    }
    return frames;
}

void addSwitch(QWidget* parent, const QStringList& frames, int initial, int x, int y,
    SwitchActuation actuation, qreal scale)
{
    const QPixmap first(frames.at(initial));
    const int width = qRound(first.width() * scale);
    const int height = qRound(first.height() * scale);
    const QPoint center(x + width / 2, y + height / 2);
    new Wag9SpriteSwitch(frames, initial, center, parent, actuation, scale);
}