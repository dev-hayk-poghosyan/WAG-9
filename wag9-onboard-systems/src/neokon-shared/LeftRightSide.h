#pragma once

#include "PlusEnum.h"
#include "LeftRightSide.pb.h"

/**
 * Used when necessary to indicate the side of the program object.
 */
enum class LeftRightSide
{
    Left = proto::LeftRightSide::LEFT,
    Right = proto::LeftRightSide::RIGHT
};

inline LeftRightSide oppositeSide(LeftRightSide side)
{
    static_assert(
        +LeftRightSide::Left == 0 &&
        +LeftRightSide::Right == 1,
        "Unexpected numeric assignment for LeftRightSide");

    return static_cast<LeftRightSide>(1 - +side);
}

inline proto::LeftRightSide::Side sideToProto(LeftRightSide side)
{
    return static_cast<proto::LeftRightSide::Side>(side);
}

inline LeftRightSide sideFromProto(proto::LeftRightSide::Side side)
{
    return static_cast<LeftRightSide>(side);
}
