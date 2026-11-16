#include "RuMetroAspectVisualStateWYYGR.h"

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::Yellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // yellowLamp 
    LampMode::Off, // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::YellowGreen>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // yellowLamp 
    LampMode::On,  // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::Green>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::White>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYGR
RuMetroAspectVisualStateWYYGR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Flashing,  // whiteInvitationLamp 
};

RuMetroAspectVisualStateWYYGR const&
RuMetroAspectVisualStateWYYGR::forState(std::optional<RuMetroAspectState> state) 
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
        case RuMetroAspectState::YellowGreen:
            return stateFor<RuMetroAspectState::YellowGreen>;
        case RuMetroAspectState::Green:
        case RuMetroAspectState::GreenMore:
            return stateFor<RuMetroAspectState::Green>;
        case RuMetroAspectState::White:
            return stateFor<RuMetroAspectState::White>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateWYYGR const defaultState;
    return defaultState;
}
