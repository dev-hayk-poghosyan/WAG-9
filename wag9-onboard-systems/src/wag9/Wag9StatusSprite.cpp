#include "Wag9StatusSprite.h"

#include <QImage>

Wag9StatusSprite::Wag9StatusSprite(QWidget* parent, bool mirrored)
    : QLabel(parent)
    , m_mirrored(mirrored)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    connect(&m_timer, &QTimer::timeout, this, &Wag9StatusSprite::step);
    hide();
}

void Wag9StatusSprite::setFrames(const QStringList& frames, int intervalMs, bool pingPong)
{
    const QString key = frames.join(QLatin1Char('|'));

    if (key == m_key && !m_key.isNull())
    {
        return;
    }

    m_timer.stop();
    m_key = key.isNull() ? QStringLiteral("") : key;
    m_frames.clear();

    for (const QString& path : frames)
    {
        QPixmap pix(path);
        if (m_mirrored)
        {
            pix = QPixmap::fromImage(pix.toImage().mirrored(true, false));
        }
        m_frames << pix;
    }

    if (m_frames.isEmpty())
    {
        hide();
        return;
    }

    m_index = 0;
    m_direction = 1;
    m_pingPong = pingPong;
    setPixmap(m_frames.first());
    setFixedSize(m_frames.first().size());
    show();

    if (m_frames.size() > 1)
    {
        m_timer.start(intervalMs);
    }
}

void Wag9StatusSprite::step()
{
    const int last = m_frames.size() - 1;

    if (!m_pingPong)
    {
        m_index = m_index < last ? m_index + 1 : 0;
    }

    else
    {
        if ((m_index == last && m_direction > 0) || (m_index == 0 && m_direction < 0))
        {
            m_direction = -m_direction;
        }
        m_index += m_direction;
    }
    setPixmap(m_frames.at(m_index));
}
