#pragma once

#include "SpeedLimitMap.pb.h"
#include <map>

struct SpeedLimitProperty
{
    std::map<int, int> forwardLimits;
    std::map<int, int> backwardLimits;

    /**
      * Returns <true> if limits in both directions are the same values.
      * Or there are no limits at all.
      */
    bool sameLimitsInBothDirections() const;

    static SpeedLimitProperty fromProto(
        proto::SpeedLimitMap::SpeedLimits const& forwardLimits,
        proto::SpeedLimitMap::SpeedLimits const& backwardLimits);

    static SpeedLimitProperty fromProto(proto::SpeedLimitMap::Interval const& interval);

    proto::SpeedLimitMap::SpeedLimits forwardLimitsToProto() const;
    proto::SpeedLimitMap::SpeedLimits backwardLimitsToProto() const;

    SpeedLimitProperty inverted() const;
};

bool operator==(SpeedLimitProperty const& lhs, SpeedLimitProperty const& rhs);
bool operator!=(SpeedLimitProperty const& lhs, SpeedLimitProperty const& rhs);
