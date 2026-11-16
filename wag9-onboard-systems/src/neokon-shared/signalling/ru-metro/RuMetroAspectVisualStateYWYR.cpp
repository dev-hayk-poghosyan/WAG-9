#include "RuMetroAspectVisualStateYWYR.h"

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::DoubleYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // yellowLamp 
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off, // blueLamp
    LampMode::Flashing, // topYellowLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // yellowLamp 
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::White>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWYR
RuMetroAspectVisualStateYWYR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // redLamp
    LampMode::Flashing,  // whiteInvitationLamp 
};

RuMetroAspectVisualStateYWYR const&
RuMetroAspectVisualStateYWYR::forState(std::optional<RuMetroAspectState> state) 
{
    if (state)
    {
        switch (*state)
        {
        case RuMetroAspectState::Blue:
            return stateFor<RuMetroAspectState::Blue>;
        case RuMetroAspectState::Red:
            return stateFor<RuMetroAspectState::Red>;
        case RuMetroAspectState::RedYellow:
            return stateFor<RuMetroAspectState::RedYellow>;
        case RuMetroAspectState::DoubleYellow:
            return stateFor<RuMetroAspectState::DoubleYellow>;
        case RuMetroAspectState::DoubleYellowTopFlashing:
            return stateFor<RuMetroAspectState::DoubleYellowTopFlashing>;
        case RuMetroAspectState::White:
            return stateFor<RuMetroAspectState::White>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateYWYR const defaultState;
    return defaultState;
}
