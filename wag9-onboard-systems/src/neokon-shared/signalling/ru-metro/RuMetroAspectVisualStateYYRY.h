#pragma once

#include "RuMetroAspectState.h"

#include "neokon-shared/signalling/LampMode.h"

struct RuMetroAspectVisualStateYYRY
{
    LampMode blueLamp = LampMode::Off;
    LampMode topYellowLamp = LampMode::Off;
    LampMode yellowLamp = LampMode::Off;
    LampMode redLamp = LampMode::Off;
    LampMode bottomYelowLamp = LampMode::Off;
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
    static RuMetroAspectVisualStateYYRY const& forState(std::optional<RuMetroAspectState> state);
};
