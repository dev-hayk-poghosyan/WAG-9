//-----------------------------------------------------------------------------
// MaterialMasks.h
//
// All commonly used material masks (some are still left in WeatherController
//
// Created by Artem Suravikin at 2019.05.23
//-----------------------------------------------------------------------------
#pragma once

namespace MaterialMasks
{

enum MaterialMasks : int
{
    MaskTerrainBit = 1 << 0,
    MaskTrain = 1 << 3,
    MaskObjectsNight = 1 << 4,
    MaskObjectsDay = 1 << 5,

    MaskFireDecal = 1 << 22,
    MaskRainOnTerrain = 1 << 23,

    MaskRainDecals = MaskRainOnTerrain | 0x2 | 0x4,
    MaskSnowDecals = ~(MaskObjectsNight | MaskObjectsDay),

    MaskTerrain = MaskTerrainBit | MaskRainOnTerrain,

    MaskEverything = 0xffffff,
};

} // namespace MaterialMasks
