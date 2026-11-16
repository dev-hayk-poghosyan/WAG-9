#pragma once

#include "LampMode.h"
#include "RuEntrance2MainAspectState.h"

#include <optional>

struct RuEntrance2MainAspectVisualState
{
    LampMode topYellowLamp = LampMode::Off;
    LampMode redLamp = LampMode::Off;
    LampMode bottomYellowLamp = LampMode::Off;

    template<RuEntrance2MainAspectState>
    static RuEntrance2MainAspectVisualState const stateFor;

    /**
     * @note If @p state is std::nullopt or otherwise invalid, a reference to a default-constructed
     *       instance of this class is returned.
     */
    static RuEntrance2MainAspectVisualState const& forState(
        std::optional<RuEntrance2MainAspectState> state);
};
