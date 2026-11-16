#include "RuMetroAspectVisualStateYYYYGR.h"

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::Off, // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::Off, // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::Off, // YellowLamp3
    LampMode::On,  // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::Yellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::On,  // YellowLamp2
    LampMode::Off, // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::YellowGreen>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::On,  // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::Green>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::Off, // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::DoubleYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::On,  // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off, // blueLamp
    LampMode::Flashing, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::On,  // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYYGR
RuMetroAspectVisualStateYYYYGR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // YellowLamp1
    LampMode::Off, // YellowLamp2
    LampMode::Off, // YellowLamp3
    LampMode::Off, // YellowLamp4 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Flashing, // whiteInvitationLamp 
};

RuMetroAspectVisualStateYYYYGR const&
RuMetroAspectVisualStateYYYYGR::forState(std::optional<RuMetroAspectState> state) 
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
        case RuMetroAspectState::DoubleYellow:
            return stateFor<RuMetroAspectState::DoubleYellow>;
        case RuMetroAspectState::DoubleYellowTopFlashing:
            return stateFor<RuMetroAspectState::DoubleYellowTopFlashing>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateYYYYGR const defaultState;
    return defaultState;
}
