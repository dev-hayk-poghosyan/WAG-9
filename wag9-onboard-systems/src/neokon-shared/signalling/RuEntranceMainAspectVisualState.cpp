#include "RuEntranceMainAspectVisualState.h"

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<RuEntranceMainAspectState::Green>
{
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // bottomYellowLamp
    false,         // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<RuEntranceMainAspectState::Yellow>
{
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // bottomYellowLamp
    false,         // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<RuEntranceMainAspectState::DoubleYellow>
{
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::On,  // topYellowLamp
    LampMode::On,  // bottomYellowLamp
    false,         // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<RuEntranceMainAspectState::FlashingYellow>
{
    LampMode::Off,      // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Flashing, // topYellowLamp
    LampMode::Off,      // bottomYellowLamp
    false,              // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<RuEntranceMainAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off,      // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Flashing, // topYellowLamp
    LampMode::On,       // bottomYellowLamp
    false,              // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<
    RuEntranceMainAspectState::DoubleYellowPlusGreenBar>
{
    LampMode::Off,  // greenLamp
    LampMode::Off,  // redLamp
    LampMode::On,   // topYellowLamp
    LampMode::On,   // bottomYellowLamp
    true,           // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<
    RuEntranceMainAspectState::DoubleYellowTopFlashingPlusGreenBar>
{
    LampMode::Off,      // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Flashing, // topYellowLamp
    LampMode::On,       // bottomYellowLamp
    true,               // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<
    RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar>
{
    LampMode::Flashing, // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Off,      // topYellowLamp
    LampMode::On,       // bottomYellowLamp
    true,               // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<
    RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars>
{
    LampMode::Flashing, // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Off,      // topYellowLamp
    LampMode::On,       // bottomYellowLamp
    true,               // greenBarsLit
};

template<>
RuEntranceMainAspectVisualState
RuEntranceMainAspectVisualState::stateFor<RuEntranceMainAspectState::Red>
{
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // bottomYellowLamp
    false,         // greenBarsLit
};

RuEntranceMainAspectVisualState const&
RuEntranceMainAspectVisualState::forState(std::optional<RuEntranceMainAspectState> state)
{
    if (state)
    {
        switch (*state)
        {
        case RuEntranceMainAspectState::Green:
            return stateFor<RuEntranceMainAspectState::Green>;
        case RuEntranceMainAspectState::Yellow:
            return stateFor<RuEntranceMainAspectState::Yellow>;
        case RuEntranceMainAspectState::DoubleYellow:
            return stateFor<RuEntranceMainAspectState::DoubleYellow>;
        case RuEntranceMainAspectState::FlashingYellow:
            return stateFor<RuEntranceMainAspectState::FlashingYellow>;
        case RuEntranceMainAspectState::DoubleYellowTopFlashing:
            return stateFor<RuEntranceMainAspectState::DoubleYellowTopFlashing>;
        case RuEntranceMainAspectState::DoubleYellowPlusGreenBar:
            return stateFor<RuEntranceMainAspectState::DoubleYellowPlusGreenBar>;
        case RuEntranceMainAspectState::DoubleYellowTopFlashingPlusGreenBar:
            return stateFor<RuEntranceMainAspectState::DoubleYellowTopFlashingPlusGreenBar>;
        case RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar:
            return stateFor<RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar>;
        case RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars:
            return stateFor<RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars>;
        case RuEntranceMainAspectState::Red:
            return stateFor<RuEntranceMainAspectState::Red>;
        }
    }

    static RuEntranceMainAspectVisualState const defaultState;
    return defaultState;
}
