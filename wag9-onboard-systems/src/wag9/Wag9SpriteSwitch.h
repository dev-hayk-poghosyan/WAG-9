#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QStringList>
#include <QVector>

enum class SwitchActuation
{
    Cycle,
    // Frames: off, pressed off, pressed on, on. Press keeps the latched state and
    // shows only its pressed frame. Release switches the state.
    Lamp
};

class Wag9SpriteSwitch  : public QLabel
{
    public:
        explicit Wag9SpriteSwitch(const QStringList& frames, int initial, const QPoint& center, QWidget* parent,
            SwitchActuation actuation = SwitchActuation::Cycle);
        virtual ~Wag9SpriteSwitch() override = default;

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        void applyFrame();
        QVector<QPixmap> m_frames;
        int m_index = 0;
        QPoint m_center;
        SwitchActuation m_actuation = SwitchActuation::Cycle;
        bool m_on = false;
};

QStringList framesOf(const QString& folder, const QString& sprite, const QList<int>& indexes);
void addSwitch(QWidget* parent, const QStringList& frames, int initial, int x, int y, SwitchActuation actuation = SwitchActuation::Cycle);