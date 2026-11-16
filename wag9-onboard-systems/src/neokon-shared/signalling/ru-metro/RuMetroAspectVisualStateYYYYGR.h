#pragma once
#include "RuMetroAspectState.h"

#include "neokon-shared/signalling/LampMode.h"

struct RuMetroAspectVisualStateYYYYGR
{
    LampMode blueLamp = LampMode::Off;
    LampMode yellowLamp1 = LampMode::Off;
    LampMode yellowLamp2 = LampMode::Off;
    LampMode yellowLamp3 = LampMode::Off;
    LampMode yellowLamp4 = LampMode::Off;
    LampMode greenLamp = LampMode::Off;
    LampMode redLamp = LampMode::Off;
    LampMode whiteInvitationLamp = LampMode::Off;

    /**
     * @note If @p state is std::nullopt or otherwise invalid, a reference to a default-constructed
     *       instance of this class is returned.
     */
    template<RuMetroAspectState>
    static RuMetroAspectState const stateFor;

    /**
     * @note If @p state is std::nullopt or otherwise invalid, a reference to a default-constructed
     *       instance of this class is returned.
     */
    static RuMetroAspectVisualStateYYYYGR const& forState(std::optional<RuMetroAspectState> state);
};
