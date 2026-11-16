#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * The Russian-style additional entrance signal typically has 3 main aspect lights:
 * TopYellow, Red and BottomYellow. Sometimes the BottomYellow may not be there.
 *
 * Such signals also typically have an invitational flashing white and a shunting
 * white, but they are not part of this state.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class RuEntrance2MainAspectState
{
    /**
     * The next section is clear.
     */
    DoubleYellow = 0,

    /**
     * The next section is closed.
     */
    Red = 1
};

std::optional<RuEntrance2MainAspectState> ruEntrance2MainAspectStateFromInt(int state);

int toProto(RuEntrance2MainAspectState state);

int toProto(std::optional<RuEntrance2MainAspectState> const& state);

int toProto(std::optional<std::optional<RuEntrance2MainAspectState>> const& state);

void fromProto(int protoVal, std::optional<RuEntrance2MainAspectState>& target);

void fromProto(int protoVal, std::optional<std::optional<RuEntrance2MainAspectState>>& target);
