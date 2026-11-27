#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QPair>
#include <QPixmap>
#include <QStringList>
#include <QTimer>
#include <QVector>

enum class SwitchActuation
{
    Cycle,
    Lamp,
    Choose,
    Indicator,
    PushIn,
    Reset       // a tripped relay (frame 1) is reset by a click on its knob: frames 2.. are played, then frame 0
};

class Wag9SpriteSwitch : public QLabel
{
    Q_OBJECT

    public:
        explicit Wag9SpriteSwitch(const QStringList& frames, int initial,
            const QPoint& center, QWidget* parent,
            SwitchActuation actuation = SwitchActuation::Cycle, qreal scale = 1);
        virtual ~Wag9SpriteSwitch() override = default;

        int position() const;
        int pressCount() const;
        void setPosition(int frame);
        void setChoices(const QVector<QPair<QString, int>>& choices);

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        void applyFrame();
        void stepAnimation();

        int m_index = 0;
        int m_pressCount = 0;
        bool m_on = false;
        QPoint m_center;
        QVector<QPixmap> m_frames;
        SwitchActuation m_actuation = SwitchActuation::Cycle;
        QVector<QPair<QString, int>> m_choices;
        QTimer m_animation;
};

QStringList framesOf(const QString& folder, const QString& sprite, int frameCount);

Wag9SpriteSwitch* addSwitch(QWidget* parent, const QStringList& frames, int initial, int x, int y,
    const QString& name = {}, SwitchActuation actuation = SwitchActuation::Cycle, qreal scale = 1);