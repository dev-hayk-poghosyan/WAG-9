#include "RuMetroAspectVisualStateWYGR.h"

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::White>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::Yellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::YellowGreen>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // yellowLamp
    LampMode::On,  // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::Green>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYGR
RuMetroAspectVisualStateWYGR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Flashing,  // whiteInvitationLamp 
};

RuMetroAspectVisualStateWYGR const&
RuMetroAspectVisualStateWYGR::forState(std::optional<RuMetroAspectState> state) 
{
    if (state)
    {
        switch (*state)
        {
        case RuMetroAspectState::Blue:
            return stateFor<RuMetroAspectState::Blue>;
        case RuMetroAspectState::Red:
            return stateFor<RuMetroAspectState::Red>;
        case RuMetroAspectState::White:
            return stateFor<RuMetroAspectState::White>;
        case RuMetroAspectState::RedYellow:
            return stateFor<RuMetroAspectState::RedYellow>;
        case RuMetroAspectState::Yellow:
            return stateFor<RuMetroAspectState::Yellow>;
        case RuMetroAspectState::YellowGreen:
            return stateFor<RuMetroAspectState::YellowGreen>;
        case RuMetroAspectState::Green:
        case RuMetroAspectState::GreenMore:
            return stateFor<RuMetroAspectState::Green>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateWYGR const defaultState;
    return defaultState;
}
