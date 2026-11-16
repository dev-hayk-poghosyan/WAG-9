#pragma once

#include "PlusEnum.h"
#include "RailPathEndpoint.pb.h"

/**
 * The distance along the rail path grows when moving from Front to Back.
 */
enum class RailPathEndpoint
{
    Front = 0,
    Back  = 1
};

static_assert(
    +RailPathEndpoint::Front == proto::RailPathEndpoint::FRONT &&
    +RailPathEndpoint::Back == proto::RailPathEndpoint::BACK,
    "RailPathEndpoint doesn't match proto::RailPathEndpoint::Endpoint");

inline RailPathEndpoint oppositeEndpoint(RailPathEndpoint endpoint)
{
    return static_cast<RailPathEndpoint>(1 - static_cast<int>(endpoint));
}

inline RailPathEndpoint endpointFromProto(proto::RailPathEndpoint::Endpoint endpoint)
{
    return static_cast<RailPathEndpoint>(endpoint);
}

inline proto::RailPathEndpoint::Endpoint protoFromEndpoint(RailPathEndpoint endpoint)
{
    return static_cast<proto::RailPathEndpoint::Endpoint>(endpoint);
}
