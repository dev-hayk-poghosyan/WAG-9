#include "AutomatonButton.h"
#include <QDebug>
#include <QSound>

AutomatonButton::AutomatonButton(QWidget* parent)
    : QPushButton(parent)
{
    setCheckable(true);

    connect(this, &QPushButton::toggled, this, [this](bool on) {
        qDebug() << "Automaton[" << objectName() << "] is " << on;
        if (!m_silentMode) {
            QSound::play(":/sounds/switch.wav");
        }
    });
}

void AutomatonButton::setSilentMode(bool silent)
{
    m_silentMode = silent;
}
