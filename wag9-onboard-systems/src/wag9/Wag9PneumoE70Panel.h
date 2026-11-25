#pragma once

#include <QLabel>
#include <QPoint>
#include <QWidget>

#include <optional>

class Wag9Needle;

class Wag9PneumoE70Panel : public QWidget
{
    public:
        explicit Wag9PneumoE70Panel(QWidget *parent = nullptr);

        void setPressure(double value);
        void setLed(const QString& name, bool on);

    private:
        void placeLed(const char* name, const QPoint& pos);

        void place(const char* name, const QString& sprite, const QPoint& pivot,
                int frameCount = 2, int initial = 0, std::optional<QPoint> hinge = std::nullopt);

        QLabel* m_picture = nullptr;
        qreal m_scale = 1;
        Wag9Needle* m_needle = nullptr;
};
