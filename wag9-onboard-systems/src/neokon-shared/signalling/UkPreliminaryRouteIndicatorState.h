#pragma once

#include "UkJunctionIndicatorArm.h"
#include "neokon-shared/PlusEnum.h"

#include <optional>
#include <string>

/**
 * The state of UK's preliminary route indicator.
 *
 * @note The numeric values are saved into the scenario.
 */
enum class UkPreliminaryRouteIndicatorState
{
    NoInidicator = 0,
    Top = 1,
    TopLeft = 2,
    Left = 3,
    BottomLeft = 4,
    TopRight = 5,
    Right = 6,
    BottomRight = 7,
};


std::optional<UkPreliminaryRouteIndicatorState> ukPreliminaryRouteIndicatorStateFromInt(int state);

UkPreliminaryRouteIndicatorState ukPreliminaryRouteIndicatorStateFromJunctionState(
    std::optional<UkJunctionIndicatorArm> arm, bool nonRedSignal);

int toProto(UkPreliminaryRouteIndicatorState state);

int toProto(std::optional<UkPreliminaryRouteIndicatorState> const& state);

int toProto(std::optional<std::optional<UkPreliminaryRouteIndicatorState>> const& state);

void fromProto(int protoVal, std::optional<UkPreliminaryRouteIndicatorState>& target);

void fromProto(int protoVal, std::optional<std::optional<UkPreliminaryRouteIndicatorState>>& target);
