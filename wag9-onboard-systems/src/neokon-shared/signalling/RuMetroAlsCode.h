#pragma once

#include "neokon-shared/signalling/LegacySignalLightState.h"
#include <optional>
#include <string>

/**
 * @note The numbers following the name indicate the speed limit in KMH.
 */
enum class RuMetroAlsCode
{
    NoFrequency0,
    AbsoluteStop0,
    Red0,
    Yellow40,
    Green60,
    Green70,
    Green75,
    Green80
};

RuMetroAlsCode ruMetroAlsCodeFromLegacySignalLightState(LegacySignalLightState state);

RuMetroAlsCode ruMetroAlsCodeFromInt(int kmh);

std::string ruMetroAlsCodeToStr(RuMetroAlsCode alsCode);

/**
 * @returns std::nullopt for NoFrequency0, 0.0 for AbsoluteStop0 signal.
 */
std::optional<double> ruMetroSpeedLimitMsFromAlsCode(RuMetroAlsCode alsCode);
