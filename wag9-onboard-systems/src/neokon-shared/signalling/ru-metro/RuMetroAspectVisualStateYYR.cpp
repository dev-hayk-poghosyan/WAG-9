#include "RuMetroAspectVisualStateYYR.h"

template<>
RuMetroAspectVisualStateYYR
RuMetroAspectVisualStateYYR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // topYellowLamp 
    LampMode::Off, // yellowLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYR
RuMetroAspectVisualStateYYR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp 
    LampMode::Off, // yellowLamp
    LampMode::On,  // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYR
RuMetroAspectVisualStateYYR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // yellowLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYR
RuMetroAspectVisualStateYYR::stateFor<RuMetroAspectState::Yellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // yellowLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYR
RuMetroAspectVisualStateYYR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Flashing,  // whiteInvitationLamp 
};

RuMetroAspectVisualStateYYR const&
RuMetroAspectVisualStateYYR::forState(std::optional<RuMetroAspectState> state) 
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
        case RuMetroAspectState::Yellow:
            return stateFor<RuMetroAspectState::Yellow>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateYYR const defaultState;
    return defaultState;
}
