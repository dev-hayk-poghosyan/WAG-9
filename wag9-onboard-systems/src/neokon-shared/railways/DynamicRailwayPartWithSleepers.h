#pragma once

#include <vector>
#include <optional>

#include <UnigineMathLib.h>

/**
 * This class is used by 'DynamicRailways' parts to get sleepers info from other parts.
 */
class DynamicRailwayPartWithSleepers
{
public:
    struct Sleeper
    {
        Unigine::Math::Mat4 worldTransform;
    };

    using Sleepers = std::vector<Sleeper>;
    using SleepersPtr = std::shared_ptr<Sleepers>;
    using ConstSleepersPtr = std::shared_ptr<Sleepers const>;

public:
    virtual ~DynamicRailwayPartWithSleepers() = default;

    virtual ConstSleepersPtr getSleepers(int pathIdx) const = 0;
};
