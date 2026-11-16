#pragma once

#include "JunctionConnection.pb.h"
#include "RailPathType.pb.h"

#include <string>

enum class RailPathType
{
    Branch = proto::PATH_BRANCH,

    YSwitchRootToLeft = proto::PATH_YSW_ROOT_TO_LEFT,
    YSwitchRootToRight = proto::PATH_YSW_ROOT_TO_RIGHT,
    
    XSwitchLeftToLeft = proto::PATH_XSW_LEFT_TO_LEFT,
    XSwitchLeftToRight = proto::PATH_XSW_LEFT_TO_RIGHT,
    XSwitchRightToLeft = proto::PATH_XSW_RIGHT_TO_LEFT,
    XSwitchRightToRight = proto::PATH_XSW_RIGHT_TO_RIGHT,

    XCrossLeftToRight = proto::PATH_XCR_LEFT_TO_RIGHT,
    XCrossRightToLeft = proto::PATH_XCR_RIGHT_TO_LEFT,

    TwSwitchRootToSide = proto::PATH_TW_ROOT_TO_SIDE,
    TwSwitchRootToMiddle = proto::PATH_TW_ROOT_TO_MIDDLE,
    TwSwitchMiddleToLeft = proto::PATH_TW_MIDDLE_TO_LEFT,
    TwSwitchMiddleToRight = proto::PATH_TW_MIDDLE_TO_RIGHT,
};

inline bool railPathTypeIsBranch(RailPathType type)
{
    return type == RailPathType::Branch;
}

bool railPathTypeIsSwitch(RailPathType type);

bool railPathTypeIsCross(RailPathType type);

RailPathType railPathTypeFromJunctionConnectionType(proto::JunctionConnection::Type type);

/**
 * Formats a RailPathType to a descriptive string for debugging purposes.
 */
std::string railPathTypeToString(RailPathType type);
