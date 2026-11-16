#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * Represents the state of a signal aspect that can be on or off.
 *
 * The advantage in using this enum over a simple boolean is the availability
 * of the overloaded fromProto() / toProto() functions.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class BooleanAspectState
{
    Off = 0,
    On = 1,
};

std::optional<BooleanAspectState> booleanAspectStateFromInt(int state);

int toProto(BooleanAspectState state);

int toProto(std::optional<BooleanAspectState> const& state);

int toProto(std::optional<std::optional<BooleanAspectState>> const& state);

void fromProto(int protoVal, std::optional<BooleanAspectState>& target);

void fromProto(int protoVal, std::optional<std::optional<BooleanAspectState>>& target);
