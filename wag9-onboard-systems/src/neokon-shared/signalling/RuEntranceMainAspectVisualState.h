#pragma once

#include "LampMode.h"
#include "RuEntranceMainAspectState.h"

#include <optional>

struct RuEntranceMainAspectVisualState
{
    LampMode greenLamp = LampMode::Off;
    LampMode redLamp = LampMode::Off;
    LampMode topYellowLamp = LampMode::Off;
    LampMode bottomYellowLamp = LampMode::Off;
    bool greenBarsLit = false;

    template<RuEntranceMainAspectState>
    static RuEntranceMainAspectVisualState const stateFor;

    /**
     * @note If @p state is std::nullopt or otherwise invalid, a reference to a default-constructed
     *       instance of this class is returned.
     */
    static RuEntranceMainAspectVisualState const& forState(
        std::optional<RuEntranceMainAspectState> state);
};
