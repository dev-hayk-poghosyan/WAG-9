#pragma once

#include <QPushButton>

/**
 * Circuit braker button. Has only two states.
 */
class AutomatonButton : public QPushButton
{
public:
    AutomatonButton(QWidget* parent = nullptr);

    /**
     * Silent mode means that no sounds will be played after state toggling.
     */
    void setSilentMode(bool silent);

private:
    bool m_silentMode = true;
};
