#include "RuMetroAspectVisualStateWYYYGR.h"

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::Green>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::On,  // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::Yellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off  // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::DoubleYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Flashing, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::On,  // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::White>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::Off, // redLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateWYYYGR
RuMetroAspectVisualStateWYYYGR::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // whiteLamp
    LampMode::Off, // topYellowLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // bottomYellowLamp 
    LampMode::Off, // greenLamp
    LampMode::On,  // redLamp
    LampMode::Flashing, // whiteInvitationLamp 
};

RuMetroAspectVisualStateWYYYGR const&
RuMetroAspectVisualStateWYYYGR::forState(std::optional<RuMetroAspectState> state) 
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

    static RuMetroAspectVisualStateWYYYGR const defaultState;
    return defaultState;
}
