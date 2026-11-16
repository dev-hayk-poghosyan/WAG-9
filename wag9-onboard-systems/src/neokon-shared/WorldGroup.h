#pragma once

#include "PlusEnum.h"
#include "WorldGroupType.pb.h"

/** Group of the world to process some exclusive cases.
 * @Default default world group. Usually applied for post-USSR states.
 * @UK United Kingdom. Tilting trains for example.
 */

enum class WorldGroup
{
    Default = 0,
    UK = 1,
    Metro = 2
};

static_assert(
    +WorldGroup::Default == proto::WorldGroupType::Default &&
    +WorldGroup::UK == proto::WorldGroupType::UK &&
    +WorldGroup::Metro == proto::WorldGroupType::Metro,
    "WorldGroup doesn't match proto::WorldGroupType::WorldGroup");


inline WorldGroup worldGroupFromProto(proto::WorldGroupType::WorldGroup group)
{
    return static_cast<WorldGroup>(group);
}

inline proto::WorldGroupType::WorldGroup protoFromWorldGroup(WorldGroup group)
{
    return static_cast<proto::WorldGroupType::WorldGroup>(group);
}
