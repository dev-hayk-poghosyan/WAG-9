#pragma once

#include "LampMode.h"
#include "RuExitMainAspectState.h"

#include <optional>

struct RuExitMainAspectStateInfo
{
    LampMode greenLamp = LampMode::Off;
    LampMode redLamp = LampMode::Off;
    LampMode topOrSingleYellowLamp = LampMode::Off;
    LampMode bottomYellowLamp = LampMode::Off;
    bool greenBarsLit = false;

    template<RuExitMainAspectState>
    static RuExitMainAspectStateInfo const info;

    /**
     * @note If @p state is std::nullopt or otherwise invalid, a reference to a default-constructed
     *       instance of RuExitMainAspectStateInfo is returned.
     */
    static RuExitMainAspectStateInfo const& forState(std::optional<RuExitMainAspectState> state);
};
