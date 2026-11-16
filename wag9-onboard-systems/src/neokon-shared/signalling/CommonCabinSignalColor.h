#pragma once

#include "neokon-shared/signalling/LegacySignalLightState.h"

#include <optional>
#include <string>

/**
 * This set of indications is common in Russia and post-Soviet contries.
 *
 * @note To be kept in sync with proto::CommonCabinSignalColor.
 */
enum class CommonCabinSignalColor
{
    Off = 0,
    Green = 1,
    Yellow = 2,
    White = 3,
    RedYellow = 4,
    Red = 5
};

CommonCabinSignalColor commonCabinSignalColorFromLegacySignalLightState(
    LegacySignalLightState state);

std::optional<CommonCabinSignalColor> commonCabinSignalColorFromUuid(std::string const& uuid);
