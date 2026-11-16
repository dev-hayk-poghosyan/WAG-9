#include "RuMetroAspectVisualStateDepoWRW.h"

template<>
RuMetroAspectVisualStateDepoWRW
RuMetroAspectVisualStateDepoWRW::stateFor<RuMetroAspectState::Red>
{
    LampMode::Off, // whiteTopLamp 
    LampMode::On,  // redLamp
    LampMode::Off, // whiteBottomLamp

    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateDepoWRW
RuMetroAspectVisualStateDepoWRW::stateFor<RuMetroAspectState::White>
{
    LampMode::On,  // whiteTopLamp 
    LampMode::Off, // redLamp
    LampMode::Off, // whiteBottomLamp

    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateDepoWRW
RuMetroAspectVisualStateDepoWRW::stateFor<RuMetroAspectState::WhiteWhite>
{
    LampMode::On,  // whiteTopLamp 
    LampMode::Off, // redLamp
    LampMode::On,  // whiteBottomLamp

    LampMode::Off, // whiteInvitationLamp 
};

template<>
RuMetroAspectVisualStateDepoWRW
RuMetroAspectVisualStateDepoWRW::stateFor<RuMetroAspectState::InvitationWhite>
{
    LampMode::Off,      // whiteTopLamp 
    LampMode::On,       // redLamp
    LampMode::Off,      // whiteBottomLamp

    LampMode::Flashing, // whiteInvitationLamp 
};

RuMetroAspectVisualStateDepoWRW const&
RuMetroAspectVisualStateDepoWRW::forState(std::optional<RuMetroAspectState> state)
{
    if (state)
    {
        switch (*state)
        {
        case RuMetroAspectState::Red:
            return stateFor<RuMetroAspectState::Red>;
        case RuMetroAspectState::White:
            return stateFor<RuMetroAspectState::White>;
        case RuMetroAspectState::WhiteWhite:
            return stateFor<RuMetroAspectState::WhiteWhite>;
        case RuMetroAspectState::InvitationWhite:
            return stateFor<RuMetroAspectState::InvitationWhite>;
        }
    }

    static RuMetroAspectVisualStateDepoWRW const defaultState;
    return defaultState;
}
