#include "ControlButtonState.h"
#include <cassert>

std::string controlButtonStateToString(ControlButtonState state)
{
    switch (state)
    {
    case ControlButtonState::Off: return "Off";
    case ControlButtonState::Intermediate:  return "Intermediate";
    case ControlButtonState::On:  return "On";
    case ControlButtonState::PressedOff:  return "PressedOff";
    case ControlButtonState::PressedOn:  return "PressedOn";
    default: assert(!"Not implemented");
    }

    return std::string();
}

bool pressedState(ControlButtonState state)
{
    return state == ControlButtonState::PressedOff || state == ControlButtonState::PressedOn;
}

bool onState(ControlButtonState state)
{
    return state == ControlButtonState::On || state == ControlButtonState::PressedOn;
}

