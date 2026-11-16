#pragma once

namespace PhysicsIntersectionMask
{
/**
 * Physics intersection mask used in TrainSim and GlobalTerrainEditor.
 */
enum PhysicsIntersectionMask
{
    TrainCollider = 1 << 0,
    InteractableObject = 1 << 1 // Objects with which the user can interact.
};
}// namespace PhysicsIntersectionMask
