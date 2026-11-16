#include "RuMetroAspectVisualStateYYRY.h"

template<>
RuMetroAspectVisualStateYYRY
RuMetroAspectVisualStateYYRY::stateFor<RuMetroAspectState::Blue>
{
    LampMode::On,  // blueLamp
    LampMode::Off, // topYellowLamp 
    LampMode::Off, // yellowLamp
    LampMode::Off, // redLamp
    LampMode::Off, // bottomYelowLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYRY
RuMetroAspectVisualStateYYRY::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp 
    LampMode::Off, // yellowLamp
    LampMode::On,  // redLamp
    LampMode::Off, // bottomYelowLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYRY
RuMetroAspectVisualStateYYRY::stateFor<RuMetroAspectState::RedYellow>
{
    LampMode::Off, // blueLamp
    LampMode::On,  // topYellowLamp
    LampMode::Off, // yellowLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // bottomYelowLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYRY
RuMetroAspectVisualStateYYRY::stateFor<RuMetroAspectState::DoubleYellow>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::On,  // yellowLamp 
    LampMode::Off, // redLamp
    LampMode::On,  // bottomYelowLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYRY
RuMetroAspectVisualStateYYRY::stateFor<RuMetroAspectState::DoubleYellowTopFlashing>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // topYellowLamp
    LampMode::Flashing, // yellowLamp 
    LampMode::Off, // redLamp
    LampMode::On,  // bottomYelowLamp
    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateYYRY
RuMetroAspectVisualStateYYRY::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off, // blueLamp
    LampMode::Off, // yellowLamp
    LampMode::Off, // greenLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // bottomYelowLamp
    LampMode::Flashing, // whiteInvitationLamp 
};

RuMetroAspectVisualStateYYRY const&
RuMetroAspectVisualStateYYRY::forState(std::optional<RuMetroAspectState> state) 
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
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateYYRY const defaultState;
    return defaultState;
}
