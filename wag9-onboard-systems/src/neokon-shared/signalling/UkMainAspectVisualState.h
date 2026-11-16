#pragma once

#include "LampMode.h"
#include "UkMainAspectState.h"

#include <optional>

struct UkMainAspectVisualState
{
    LampMode topYellowLamp = LampMode::Off;
    LampMode greenLamp = LampMode::Off;

    /**
     * 3-aspect signals have only one yellow lamp. 4-aspect ones have a top and a bottom one.
     * In 4-aspect signals, UkMainAspectState::Yellow corresponds to the bottom yellow lamp
     * being on.
     */
    LampMode bottomOrSoleYellowLamp = LampMode::Off;

    LampMode redLamp = LampMode::Off;

    template<UkMainAspectState>
    static UkMainAspectVisualState const stateFor;

    /**
     * @note If @p state is std::nullopt or otherwise invalid, a reference to a default-constructed
     *       instance of this class is returned.
     */
    static UkMainAspectVisualState const& forState(std::optional<UkMainAspectState> state);
};
