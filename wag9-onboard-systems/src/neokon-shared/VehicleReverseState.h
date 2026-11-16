#pragma once

#include "neokon-shared/PlusEnum.h"

enum class VehicleReverseState
{
    NonReversed,
    Reversed
};

inline VehicleReverseState invertedVehicleReverseState(VehicleReverseState value)
{
    static_assert(+VehicleReverseState::NonReversed == 0);
    static_assert(+VehicleReverseState::Reversed == 1);

    return static_cast<VehicleReverseState>((+VehicleReverseState::Reversed) - (+value));
}
