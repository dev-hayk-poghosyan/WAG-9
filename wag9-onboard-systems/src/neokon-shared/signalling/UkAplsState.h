#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * The state of UK's associated position signal light.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class UkAplsState
{
    ObeyMainSignal = 0,
    ProceedCautiously = 1,
};

std::optional<UkAplsState> ukAplsStateFromInt(int state);

int toProto(UkAplsState state);

int toProto(std::optional<UkAplsState> const& state);

int toProto(std::optional<std::optional<UkAplsState>> const& state);

void fromProto(int protoVal, std::optional<UkAplsState>& target);

void fromProto(int protoVal, std::optional<std::optional<UkAplsState>>& target);
