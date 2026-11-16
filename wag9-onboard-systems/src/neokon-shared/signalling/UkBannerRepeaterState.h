#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>
#include <string>

/**
 * The state of UK's banner repeater signal.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class UkBannerRepeaterState
{
    On = 0,
    Off = 1,
    Green = 2 // This one is only valid for 3-state banner repeaters.
};

/**
 * Returns symbol name that can be passed to SymbolMaterials::materialForSymbol().
 */
std::wstring materialSymbolFor(UkBannerRepeaterState state);

/**
 * Returns billboard symbol name that can be passed to SymbolMaterials::materialForSymbol().
 */
std::wstring materialBillboardSymbolFor(UkBannerRepeaterState state);

std::optional<UkBannerRepeaterState> ukBannerRepeaterStateFromInt(int state);

int toProto(UkBannerRepeaterState state);

int toProto(std::optional<UkBannerRepeaterState> const& state);

int toProto(std::optional<std::optional<UkBannerRepeaterState>> const& state);

void fromProto(int protoVal, std::optional<UkBannerRepeaterState>& target);

void fromProto(int protoVal, std::optional<std::optional<UkBannerRepeaterState>>& target);
