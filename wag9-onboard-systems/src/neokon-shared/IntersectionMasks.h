#pragma once

namespace IntersectionMasks
{

/**
 * All intersection masks used in both TrainSim and GlobalTerrainEditor projects.
 */
enum IntersectionMasks : int
{
    InteractableObjects = 1 << 0, // used in TrainSim
    Landscape = 1 << 1, // used in both TrainSim and GlobalTerrainEditor
    Buildings = 1 << 2, // used in GlobalTerrainEditor
    Railways = 1 << 3, // used in both TrainSim and GlobalTerrainEditor
    CrossingObjects = 1 << 4, // used in TrainSim
    Perrons = 1 << 5, // used in both TrainSim and GlobalTerrainEditor
    Widgets = 1 << 6, // used in GlobalTerrainEditor
    RailwayTies = 1 << 7, // used in TrainSim
    ConductorRail = 1 << 8, // used in TrainSim
    ObjectsOnPath = 1 << 9, // used in TrainSim

    MaskGroundIntersection = Landscape | Railways,
    MaskCharacterMovement = Landscape | Perrons,
};

} // namespace IntersectionMasks
