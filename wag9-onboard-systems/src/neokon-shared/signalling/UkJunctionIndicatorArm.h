#pragma once

#include "neokon-shared/PlusEnum.h"

#include <optional>
#include <string>

/**
 * Represents one of the arms of a UK junction indicator.
 *
 * @note The numeric values may be saved into the scenario.
 */
enum class UkJunctionIndicatorArm
{
    TopLeft = 0,
    Left = 1,
    BottomLeft = 2,
    TopRight = 3,
    Right = 4,
    BottomRight = 5
};

std::optional<UkJunctionIndicatorArm> ukJunctionIndicatorArmFromInt(int state);

std::optional<UkJunctionIndicatorArm> ukJunctionIndicatorArmFromLabel(std::wstring const& label);

std::optional<const char*> ukJunctionIndicatorArmToNameSuffix(int state);

int toProto(UkJunctionIndicatorArm state);

int toProto(std::optional<UkJunctionIndicatorArm> const& state);

int toProto(std::optional<std::optional<UkJunctionIndicatorArm>> const& state);

void fromProto(int protoVal, std::optional<UkJunctionIndicatorArm>& target);

void fromProto(int protoVal, std::optional<std::optional<UkJunctionIndicatorArm>>& target);
