#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * The sate of the UK RightAway (RA) signal.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class UkRightAwayState
{
    Off = 0,
    On = 2,
    CD = 4,
};

std::optional<UkRightAwayState> ukRightAwayStateFromInt(int state);

int toProto(UkRightAwayState state);

int toProto(std::optional<UkRightAwayState> const& state);

int toProto(std::optional<std::optional<UkRightAwayState>> const& state);

void fromProto(int protoVal, std::optional<UkRightAwayState>& target);

void fromProto(int protoVal, std::optional<std::optional<UkRightAwayState>>& target);
