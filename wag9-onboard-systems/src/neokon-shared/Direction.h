#pragma once

#include "Direction.pb.h"

/**
 * Corresponds to Direction message and Direction.Type enumeration defined in Direction.proto.
 */
enum class Direction
{
    Forward = proto::Direction::FORWARD,
    Backward = proto::Direction::BACKWARD
};

Direction reverseDirection(Direction dir);

inline Direction directionFromProto(proto::Direction::Type endpoint)
{
    return static_cast<Direction>(endpoint);
}

inline proto::Direction::Type protoFromDirection(Direction endpoint)
{
    return static_cast<proto::Direction::Type>(endpoint);
}
