#include "RuMetroAspectVisualStateYYYGR.h"

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::Green>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};


template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::Yellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::YellowGreen>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::DoubleYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // topYellowLamp
    LampMode::On,  // yellowLamp
    LampMode::Off,  // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off, // blueLamp
    LampMode::Flashing, // topYellowLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::White>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYYGR
RuMetroAspectVisualStateYYYGR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Flashing,  // whiteInvitationLamp 
};

RuMetroAspectVisualStateYYYGR const&
RuMetroAspectVisualStateYYYGR::forState(std::optional<RuMetroAspectState> state) 
{
    if (state)
    {
        switch (*state)
        {
        case RuMetroAspectState::Blue:
            return stateFor<RuMetroAspectState::Blue>;
        case RuMetroAspectState::Red:
            return stateFor<RuMetroAspectState::Red>;
        case RuMetroAspectState::Green:
        case RuMetroAspectState::GreenMore:
            return stateFor<RuMetroAspectState::Green>;
        case RuMetroAspectState::RedYellow:
            return stateFor<RuMetroAspectState::RedYellow>;
        case RuMetroAspectState::Yellow:
            return stateFor<RuMetroAspectState::Yellow>;
        case RuMetroAspectState::YellowGreen:
            return stateFor<RuMetroAspectState::YellowGreen>;
        case RuMetroAspectState::DoubleYellow:
            return stateFor<RuMetroAspectState::DoubleYellow>;
        case RuMetroAspectState::DoubleYellowTopFlashing:
            return stateFor<RuMetroAspectState::DoubleYellowTopFlashing>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateYYYGR const defaultState;
    return defaultState;
}
