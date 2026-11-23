#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QStringList>
#include <QVector>

enum class SwitchActuation
{
    Cycle,
    Lamp
};

class Wag9SpriteSwitch : public QLabel
{
    Q_OBJECT
    public:
        explicit Wag9SpriteSwitch(const QStringList& frames, int initial, const QPoint& center, QWidget* parent,
            SwitchActuation actuation = SwitchActuation::Cycle, qreal scale = 1);
        virtual ~Wag9SpriteSwitch() override = default;

        int position() const;

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;

    private:
        void applyFrame();

        int m_index = 0;
        bool m_on = false;
        QPoint m_center;
        QVector<QPixmap> m_frames;
        SwitchActuation m_actuation = SwitchActuation::Cycle;
};

QStringList framesOf(const QString& folder, const QString& sprite, const QList<int>& indexes);

void addSwitch(QWidget* parent, const QStringList& frames, int initial, int x, int y,
    const QString& name = {}, SwitchActuation actuation = SwitchActuation::Cycle, qreal scale = 1);