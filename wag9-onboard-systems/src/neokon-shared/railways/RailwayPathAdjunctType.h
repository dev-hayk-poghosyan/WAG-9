#pragma once

#include "RailPathAdjunct.pb.h"
#include "neokon-shared/PlusEnum.h"

/**
 * @see RailPathAdjunct.proto
 */
enum class RailwayPathAdjunctType: int
{
    Default = proto::RailPathAdjunct::Type::RailPathAdjunct_Type_DEFAULT
};

inline RailwayPathAdjunctType
railwayPathAdjunctTypeFromProto(proto::RailPathAdjunct::Type type)
{
    static_assert(+RailwayPathAdjunctType::Default == 
                  +proto::RailPathAdjunct::Type::RailPathAdjunct_Type_DEFAULT);

    return static_cast<RailwayPathAdjunctType>(type);
}

inline proto::RailPathAdjunct::Type
railwayPathAdjunctTypeToProto(RailwayPathAdjunctType type)
{
    static_assert(+RailwayPathAdjunctType::Default == 
                  +proto::RailPathAdjunct::Type::RailPathAdjunct_Type_DEFAULT);

    return static_cast<proto::RailPathAdjunct::Type>(type);
}
