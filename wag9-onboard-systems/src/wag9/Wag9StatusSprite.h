#pragma once

#include <QLabel>
#include <QPixmap>
#include <QStringList>
#include <QTimer>
#include <QVector>

class Wag9StatusSprite : public QLabel
{
    public:
        explicit Wag9StatusSprite(QWidget* parent, bool mirrored = false);

        void setFrames(const QStringList& frames, int intervalMs = 100, bool pingPong = false);

    private:
        void step();

        QString m_key;
        QVector<QPixmap> m_frames;
        int m_index = 0;
        int m_direction = 1;
        bool m_pingPong = false;
        bool m_mirrored = false;
        QTimer m_timer;
};
