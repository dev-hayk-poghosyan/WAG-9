#pragma once

#include <QPoint>
#include <QWidget>

class Wag9Needle;

class Wag9PneumoPanel : public QWidget
{
    public:
        explicit Wag9PneumoPanel(QWidget* parent = nullptr);

        void setPressure(double value);

    private:
        void place(const char *name, const QString& sprite, const QPoint& topLeft,
                int frameCount = 2, int initial = 0);

        Wag9Needle* m_needle = nullptr;
};
