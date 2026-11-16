#include "UkMainAspectVisualState.h"

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::Green>
{
    LampMode::Off, // topYellowLamp
    LampMode::On,  // greenLamp
    LampMode::Off, // bottomOrSoleYellowLamp
    LampMode::Off, // redLamp
};

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::Yellow>
{
    LampMode::Off, // topYellowLamp
    LampMode::Off, // greenLamp
    LampMode::On,  // bottomOrSoleYellowLamp
    LampMode::Off, // redLamp
};

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::Red>
{
    LampMode::Off, // topYellowLamp
    LampMode::Off, // greenLamp
    LampMode::Off, // bottomOrSoleYellowLamp
    LampMode::On,  // redLamp
};

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::DoubleYellow>
{
    LampMode::On,  // topYellowLamp
    LampMode::Off, // greenLamp
    LampMode::On,  // bottomOrSoleYellowLamp
    LampMode::Off, // redLamp
};

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::FlashingYellow>
{
    LampMode::Off,      // topYellowLamp
    LampMode::Off,      // greenLamp
    LampMode::Flashing, // bottomOrSoleYellowLamp
    LampMode::Off,      // redLamp
};

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::FlashingDoubleYellow>
{
    LampMode::Flashing, // topYellowLamp
    LampMode::Off,      // greenLamp
    LampMode::Flashing, // bottomOrSoleYellowLamp
    LampMode::Off,      // redLamp
};

template<>
UkMainAspectVisualState
UkMainAspectVisualState::stateFor<UkMainAspectState::Off>
{
    LampMode::Off, // topYellowLamp
    LampMode::Off, // greenLamp
    LampMode::Off, // bottomOrSoleYellowLamp
    LampMode::Off, // redLamp
};

UkMainAspectVisualState const&
UkMainAspectVisualState::forState(std::optional<UkMainAspectState> state)
{
    if (state)
    {
        switch (*state)
        {
        case UkMainAspectState::Green:
            return stateFor<UkMainAspectState::Green>;
        case UkMainAspectState::Yellow:
            return stateFor<UkMainAspectState::Yellow>;
        case UkMainAspectState::Red:
            return stateFor<UkMainAspectState::Red>;
        case UkMainAspectState::DoubleYellow:
            return stateFor<UkMainAspectState::DoubleYellow>;
        case UkMainAspectState::FlashingYellow:
            return stateFor<UkMainAspectState::FlashingYellow>;
        case UkMainAspectState::FlashingDoubleYellow:
            return stateFor<UkMainAspectState::FlashingDoubleYellow>;
        case UkMainAspectState::Off:
            return stateFor<UkMainAspectState::Off>;
        }
    }

    static UkMainAspectVisualState const defaultVisualState;
    return defaultVisualState;
}
