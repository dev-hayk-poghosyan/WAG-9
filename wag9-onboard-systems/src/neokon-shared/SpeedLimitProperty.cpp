#include "SpeedLimitProperty.h"

#include <optional>

bool
SpeedLimitProperty::sameLimitsInBothDirections() const
{
    std::optional<int> lastLimit;
    for (auto const& iter : forwardLimits)
    {
        if (lastLimit && lastLimit != iter.second)
            return false;

        lastLimit = iter.second;
    }

    for (auto const& iter : backwardLimits)
    {
        if (lastLimit && lastLimit != iter.second)
            return false;

        lastLimit = iter.second;
    }

    return true;
}

SpeedLimitProperty
SpeedLimitProperty::fromProto(proto::SpeedLimitMap::SpeedLimits const& forwardLimits,
    proto::SpeedLimitMap::SpeedLimits const& backwardLimits)
{
    SpeedLimitProperty result;

    std::copy(forwardLimits.limit_by_type().begin(), forwardLimits.limit_by_type().end(),
        std::inserter(result.forwardLimits, result.forwardLimits.end()));

    std::copy(backwardLimits.limit_by_type().begin(), backwardLimits.limit_by_type().end(),
        std::inserter(result.backwardLimits, result.backwardLimits.end()));

    return result;
}

SpeedLimitProperty
SpeedLimitProperty::fromProto(proto::SpeedLimitMap::Interval const& interval)
{
    return fromProto(interval.forward_direction_limits(), interval.backward_direction_limits());
}

proto::SpeedLimitMap::SpeedLimits
SpeedLimitProperty::forwardLimitsToProto() const
{
    proto::SpeedLimitMap::SpeedLimits result;
    auto& limits = *result.mutable_limit_by_type();

    for (auto[type, limit] : forwardLimits)
    {
        limits[type] = limit;
    }
    return result;
}

proto::SpeedLimitMap::SpeedLimits
SpeedLimitProperty::backwardLimitsToProto() const
{
    proto::SpeedLimitMap::SpeedLimits result;
    auto& limits = *result.mutable_limit_by_type();

    for (auto[type, limit] : backwardLimits)
    {
        limits[type] = limit;
    }
    return result;
}

SpeedLimitProperty
SpeedLimitProperty::inverted() const
{
    return SpeedLimitProperty
    {
        backwardLimits,
        forwardLimits
    };
}

bool
operator==(SpeedLimitProperty const& lhs, SpeedLimitProperty const& rhs)
{
    return lhs.forwardLimits == rhs.forwardLimits && lhs.backwardLimits == rhs.backwardLimits;
}

bool
operator!=(SpeedLimitProperty const& lhs, SpeedLimitProperty const& rhs)
{
    return !(lhs == rhs);
}
