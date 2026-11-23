#pragma once

#include <QPixmap>
#include <QWidget>

constexpr qreal kPantoGaugeMax = 10;
constexpr qreal kPantoGaugeSweep = 270;

class Wag9Needle : public QWidget
{
    Q_OBJECT
    public:
        Wag9Needle(const QPixmap& pixmap, const QPoint& center,
            qreal maxValue, qreal sweep, QWidget* parent);

        void setValue(qreal value);

    protected:
        virtual void paintEvent(QPaintEvent* event) override;

    private:
        QPixmap m_pixmap;
        qreal m_maxValue = 1;
        qreal m_sweep = 0;
        qreal m_angle = 0;
};