#include "Wag9Needle.h"

#include <QPainter>

Wag9Needle::Wag9Needle(const QPixmap& pixmap, const QPoint& center,
    qreal maxValue, qreal sweep, QWidget *parent)
    : QWidget(parent)
    , m_pixmap(pixmap)
    , m_maxValue(maxValue)
    , m_sweep(sweep)
{
    setFixedSize(pixmap.size());
    move(center.x() - width() / 2, center.y() - height() / 2);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void Wag9Needle::setValue(qreal value)
{
    const qreal angle = qBound<qreal>(0, value, m_maxValue) / m_maxValue * m_sweep;

    if (!qFuzzyCompare(angle + 1, m_angle + 1))
    {
        m_angle = angle;
        update();
    }
}

void Wag9Needle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.translate(width() / 2.0, height() / 2.0);
    painter.rotate(m_angle);
    painter.drawPixmap(QPointF(-m_pixmap.width() / 2.0, -m_pixmap.height() / 2.0), m_pixmap);
}
