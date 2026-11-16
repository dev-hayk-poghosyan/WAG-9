#pragma once

#include <optional>

/**
 * Previously, this enum held all possible signal light states, from all parts of the world.
 *
 * Now it's used for old signal lights only.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class LegacySignalLightState
{
    Off = 0,
    Blue = 1,
    Err01 = 2,
    Green = 3,
    GreenWhite = 4,
    Red = 5,
    RedWhiteFlash = 6,
    White = 7,
    WhiteFlash = 8,
    WhiteWhite = 9,
    Yellow = 10,
    YellowFlash = 11,
    YellowYellow = 12,
    YellowYellowFlash = 13,
    RedWhite = 14,
};

std::optional<LegacySignalLightState> legacySignalLightStateFromInt(int state);

int toProto(LegacySignalLightState state);

int toProto(std::optional<LegacySignalLightState> const& state);

int toProto(std::optional<std::optional<LegacySignalLightState>> const& state);

void fromProto(int protoVal, std::optional<LegacySignalLightState>& target);

void fromProto(int protoVal, std::optional<std::optional<LegacySignalLightState>>& target);
