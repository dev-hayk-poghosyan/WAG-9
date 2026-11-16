#pragma once

#include "LeftRightSide.h"
#include "RailPathEndpoint.h"
#include "RailPathSide.h"

/**
 * Calculates whether a given RailPathSide will be on the left or the right
 * side of an observer that's standing on the RailPath in question and looking
 * along the RailPath towards the specified path endpoint.
 */
inline LeftRightSide railPathSideFromObserversPointOfView(
    RailPathSide railPathSide, RailPathEndpoint observerFacing)
{
    return (observerFacing == RailPathEndpoint::Back) ==
        (railPathSide == RailPathSide::LeftSideAlongRailPath)
        ? LeftRightSide::Left : LeftRightSide::Right;
}
