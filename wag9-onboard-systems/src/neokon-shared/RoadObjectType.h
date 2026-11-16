#pragma once

#include "PlusEnum.h"

/**
 * @note These values are sent between TranSim and RailroadControl, so be careful
 *       when changing them.
 */
enum class RoadObjectType
{
    Animal            = 0,
    Obstacle          = 1,
    //WeatherCondition  = 2, No longer used.
    Signaller         = 3,
    EnvironmentHazard = 4,
    Fault             = 5,
    AdhesionZone      = 6,
    RailSign          = 7,
    SafetyObject      = 8,
    Teleport          = 9,
    FallingPassenger  = 10,
    AnimalGroup       = 11,
    SignalStateTrigger = 12,
};
