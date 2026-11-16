#include "RailPathType.h"

#include <boost/format.hpp>

#include <stdexcept>

bool railPathTypeIsSwitch(RailPathType type)
{
    switch (type)
    {
    case RailPathType::YSwitchRootToLeft:
    case RailPathType::YSwitchRootToRight:
    case RailPathType::XSwitchLeftToLeft:
    case RailPathType::XSwitchLeftToRight:
    case RailPathType::XSwitchRightToLeft:
    case RailPathType::XSwitchRightToRight:
    case RailPathType::TwSwitchRootToSide:
    case RailPathType::TwSwitchRootToMiddle:
    case RailPathType::TwSwitchMiddleToLeft:
    case RailPathType::TwSwitchMiddleToRight:
        return true;
    default:
        return false;
    }
}

bool railPathTypeIsCross(RailPathType type)
{
    switch (type)
    {
    case RailPathType::XCrossLeftToRight:
    case RailPathType::XCrossRightToLeft:
        return true;
    default:
        return false;
    }
}

RailPathType railPathTypeFromJunctionConnectionType(proto::JunctionConnection::Type type)
{
    switch (type)
    {
    case proto::JunctionConnection::YSW_ROOT_TO_LEFT:
        return RailPathType::YSwitchRootToLeft;
    case proto::JunctionConnection::YSW_ROOT_TO_RIGHT:
        return RailPathType::YSwitchRootToRight;
    case proto::JunctionConnection::XSW_LEFT_TO_LEFT:
        return RailPathType::XSwitchLeftToLeft;
    case proto::JunctionConnection::XSW_LEFT_TO_RIGHT:
        return RailPathType::XSwitchLeftToRight;
    case proto::JunctionConnection::XSW_RIGHT_TO_LEFT:
        return RailPathType::XSwitchRightToLeft;
    case proto::JunctionConnection::XSW_RIGHT_TO_RIGHT:
        return RailPathType::XSwitchRightToRight;
    case proto::JunctionConnection::XCR_LEFT_TO_RIGHT:
        return RailPathType::XCrossLeftToRight;
    case proto::JunctionConnection::XCR_RIGHT_TO_LEFT:
        return RailPathType::XCrossRightToLeft;
    case proto::JunctionConnection::TW_ROOT_TO_SIDE:
        return RailPathType::TwSwitchRootToSide;
    case proto::JunctionConnection::TW_ROOT_TO_MIDDLE:
        return RailPathType::TwSwitchRootToMiddle;
    case proto::JunctionConnection::TW_MIDDLE_TO_LEFT:
        return RailPathType::TwSwitchMiddleToLeft;
    case proto::JunctionConnection::TW_MIDDLE_TO_RIGHT:
        return RailPathType::TwSwitchMiddleToRight;
    }

    throw std::logic_error(str(boost::format(
        "Unknown JunctionType %d") % type));
}

std::string railPathTypeToString(RailPathType type)
{
    switch (type)
    {
    case RailPathType::Branch:
        return "Branch";
    case RailPathType::YSwitchRootToLeft:
        return "YSwitchRootToLeft";
    case RailPathType::YSwitchRootToRight:
        return "YSwitchRootToRight";
    case RailPathType::XSwitchLeftToLeft:
        return "XSwitchLeftToLeft";
    case RailPathType::XSwitchLeftToRight:
        return "XSwitchLeftToRight";
    case RailPathType::XSwitchRightToLeft:
        return "XSwitchRightToLeft";
    case RailPathType::XSwitchRightToRight:
        return "XSwitchRightToRight";
    case RailPathType::XCrossLeftToRight:
        return "XCrossLeftToRight";
    case RailPathType::XCrossRightToLeft:
        return "XCrossRightToLeft";
    case RailPathType::TwSwitchRootToSide:
        return "TwSwitchRootToSide";
    case RailPathType::TwSwitchRootToMiddle:
        return "TwSwitchRootToMiddle";
    case RailPathType::TwSwitchMiddleToLeft:
        return "TwSwitchMiddleToLeft";
    case RailPathType::TwSwitchMiddleToRight:
        return "TwSwitchMiddleToRight";
    }

    return "<unknown>";
}
