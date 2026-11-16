#pragma once

#include "PlusEnum.h"
#include "RailPathSide.pb.h"

enum class RailPathSide
{
    /**
     * The left side of an observer that's facing the increasing RailPath distances.
     */
    LeftSideAlongRailPath = proto::LEFT_SIDE_ALONG_RAIL_PATH,

    /**
     * The right side of an observer that's facing the increasing RailPath distances.
     */
    RightSideAlongRailPath = proto::RIGHT_SIDE_ALONG_RAIL_PATH,
};

inline RailPathSide oppositeRailPathSide(RailPathSide railPathSide)
{
    static_assert(
        +RailPathSide::LeftSideAlongRailPath == 0 &&
        +RailPathSide::RightSideAlongRailPath == 1,
        "Unexpected numeric assignment for RailPathSide");

    return static_cast<RailPathSide>(1 - +railPathSide);
}
