#include "RailPathId.h"

#include <boost/format.hpp>
#include <boost/functional/hash.hpp>

RailPathId::RailPathId(int roadSetEntityId, RailPathType railPathType)
    : m_roadSetEntityId(roadSetEntityId)
    , m_railPathType(railPathType)
{
}

RailPathId::RailPathId(proto::RoadSegment const& roadSegment)
    : m_roadSetEntityId(roadSegment.id())
    , m_railPathType(static_cast<RailPathType>(roadSegment.type()))
{
}

proto::RoadSegment
RailPathId::toProtoSegment() const
{
    proto::RoadSegment protoSeg;

    protoSeg.set_id(m_roadSetEntityId);
    protoSeg.set_type(static_cast<proto::RailPathType>(m_railPathType));

    return protoSeg;
}

std::string
RailPathId::toString() const
{
    if (railPathTypeIsBranch(m_railPathType))
    {
        return str(boost::format("Branch %d") % m_roadSetEntityId);
    }
    else if (railPathTypeIsSwitch(m_railPathType))
    {
        return str(boost::format("Switch %d, pathType %s")
            % m_roadSetEntityId % railPathTypeToString(m_railPathType));
    }
    else if (railPathTypeIsCross(m_railPathType))
    {
        return str(boost::format("Cross %d, pathType %s")
            % m_roadSetEntityId % railPathTypeToString(m_railPathType));
    }
    else
    {
        return "<unknown>";
    }
}

size_t
RailPathId::hashValue() const noexcept
{
    size_t seed = 0;

    boost::hash_combine(seed, m_roadSetEntityId);
    boost::hash_combine(seed, m_railPathType);

    return seed;
}
