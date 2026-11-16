#include "RuMetroAspectVisualStateYWR.h"

template<>
RuMetroAspectVisualStateYWR
RuMetroAspectVisualStateYWR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // whiteLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWR
RuMetroAspectVisualStateYWR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // whiteLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWR
RuMetroAspectVisualStateYWR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // whiteLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWR
RuMetroAspectVisualStateYWR::stateFor<RuMetroAspectState::White>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // whiteLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYWR
RuMetroAspectVisualStateYWR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // whiteLamp 
    LampMode::On,  // redLamp
    LampMode::Flashing,  // whiteInvitationLamp 
};

RuMetroAspectVisualStateYWR const&
RuMetroAspectVisualStateYWR::forState(std::optional<RuMetroAspectState> state) 
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
        case RuMetroAspectState::White:
            return stateFor<RuMetroAspectState::White>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateYWR const defaultState;
    return defaultState;
}
