#include "RuExitMainAspectStateInfo.h"

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::Green>
{
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // topOrSingleYellowLamp
    LampMode::Off, // bottomYellowLamp
    false,         // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::Yellow>
{
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::On,  // topOrSingleYellowLamp
    LampMode::Off, // bottomYellowLamp
    false,         // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::DoubleYellow>
{
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::On,  // topOrSingleYellowLamp
    LampMode::On,  // bottomYellowLamp
    false,         // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::DoubleYellowPlusGreenBar>
{
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::On,  // topOrSingleYellowLamp
    LampMode::On,  // bottomYellowLamp
    true,          // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::DoubleYellowPlusTwoGreenBars>
{
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::On,  // topOrSingleYellowLamp
    LampMode::On,  // bottomYellowLamp
    true,          // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off,      // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Flashing, // topOrSingleYellowLamp
    LampMode::On,       // bottomYellowLamp
    false,              // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar>
{
    LampMode::Flashing, // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Off,      // topOrSingleYellowLamp
    LampMode::On,       // bottomYellowLamp
    true,               // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<
    RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars>
{
    LampMode::Flashing, // greenLamp
    LampMode::Off,      // redLamp
    LampMode::Off,      // topOrSingleYellowLamp
    LampMode::On,       // bottomYellowLamp
    true,               // greenBarsLit
};

template<>
RuExitMainAspectStateInfo
RuExitMainAspectStateInfo::info<RuExitMainAspectState::Red>
{
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off, // topOrSingleYellowLamp
    LampMode::Off, // bottomYellowLamp
    false,         // greenBarsLit
};

RuExitMainAspectStateInfo const&
RuExitMainAspectStateInfo::forState(std::optional<RuExitMainAspectState> state)
{
    if (state)
    {
        switch (*state)
        {
        case RuExitMainAspectState::Green:
            return info<RuExitMainAspectState::Green>;
        case RuExitMainAspectState::Yellow:
            return info<RuExitMainAspectState::Yellow>;
        case RuExitMainAspectState::DoubleYellow:
            return info<RuExitMainAspectState::DoubleYellow>;
        case RuExitMainAspectState::DoubleYellowPlusGreenBar:
            return info<RuExitMainAspectState::DoubleYellowPlusGreenBar>;
        case RuExitMainAspectState::DoubleYellowPlusTwoGreenBars:
            return info<RuExitMainAspectState::DoubleYellowPlusTwoGreenBars>;
        case RuExitMainAspectState::DoubleYellowTopFlashing:
            return info<RuExitMainAspectState::DoubleYellowTopFlashing>;
        case RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar:
            return info<RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar>;
        case RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars:
            return info<RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars>;
        case RuExitMainAspectState::Red:
            return info<RuExitMainAspectState::Red>;
        }
    }

    static RuExitMainAspectStateInfo const defaultState;
    return defaultState;
}
