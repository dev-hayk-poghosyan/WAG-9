#include "RuEntrance2MainAspectVisualState.h"

template<>
RuEntrance2MainAspectVisualState
RuEntrance2MainAspectVisualState::stateFor<RuEntrance2MainAspectState::DoubleYellow>
{
    LampMode::On,  // topYellowLamp
    LampMode::Off, // redLamp
    LampMode::On,  // bottomYellowLamp
};

template<>
RuEntrance2MainAspectVisualState
RuEntrance2MainAspectVisualState::stateFor<RuEntrance2MainAspectState::Red>
{
    LampMode::Off, // topYellowLamp
    LampMode::On,  // redLamp
    LampMode::Off, // bottomYellowLamp
};

RuEntrance2MainAspectVisualState const&
RuEntrance2MainAspectVisualState::forState(
    std::optional<RuEntrance2MainAspectState> state)
{
    if (state)
    {
        switch (*state)
        {
        case RuEntrance2MainAspectState::DoubleYellow:
            return stateFor<RuEntrance2MainAspectState::DoubleYellow>;
        case RuEntrance2MainAspectState::Red:
            return stateFor<RuEntrance2MainAspectState::Red>;
        }
    }

    static RuEntrance2MainAspectVisualState const defaultVisualState;
    return defaultVisualState;
}
