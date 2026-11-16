#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>

/**
 * All possible signal states for UK 3 or 4-aspect signal lights.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class UkMainAspectState
{
    Green = 0,
    Yellow = 1,
    Red = 2,
    DoubleYellow = 3,
    FlashingYellow = 4,
    FlashingDoubleYellow = 5,
    Off = 6,
};

std::optional<UkMainAspectState> ukMainAspectStateFromInt(int state);

bool validForUk2AspectSignal(UkMainAspectState state);

bool validForUk3AspectSignal(UkMainAspectState state);

bool validForUk4AspectSignal(UkMainAspectState state);

int toProto(UkMainAspectState state);

int toProto(std::optional<UkMainAspectState> const& state);

int toProto(std::optional<std::optional<UkMainAspectState>> const& state);

int symbolicIndicatorToProto(std::optional<int> const& state);

int manualSymbolicIndicatorToProto(std::optional<std::optional<int>> const& state);

void fromProto(int protoVal, std::optional<UkMainAspectState>& target);

void fromProto(int protoVal, std::optional<std::optional<UkMainAspectState>>& target);

void symbolicIndicatorFromProto(int protoVal, std::optional<int>& target);

void manualSymbolicIndicatorFromProto(int protoVal, std::optional<std::optional<int>>& target);
