#pragma once

#include <string>

/**
 * States of controll elements(buttons, switchers, togglers, lamps etc) on the 80x panel.
 */
enum class ControlButtonState
{
    Indeterminate,

    /**
     * Element's light indication is OFF and it's in normal(not pressed) state.
     */
    Off,

    /**
     * Element's light indication is ON and it's in normal(not pressed) state.
     */ 
    On,

    /**
     * Element's light indication is OFF and it's in pressed state.
     */
    PressedOff,
    /**
     * Element's light indication is ON and it's in pressed state.
     */
    PressedOn,

    /**
     * Element is in intermediate state.
     */
    Intermediate,

    /**
     * Some elements are covered with a cup to avoid accidental press.
     */
    CupOpened,
};

std::string controlButtonStateToString(ControlButtonState state);

bool pressedState(ControlButtonState state);

bool onState(ControlButtonState state);
