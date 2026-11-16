#pragma once

#include "RailPathType.h"
#include "RoadSegment.pb.h"

#include <cstddef>
#include <functional>
#include <string>
#include <tuple>

class RailPathId
{
public:
    RailPathId(int roadSetEntityId, RailPathType railPathType);

    /**
     * @throw std::exception If road segment type couldn't be resolved.
     *        This doesn't happen in practice.
     */
    RailPathId(proto::RoadSegment const& roadSegment);

    /**
    * @throw std::exception If rail path type isn't supported by proto::RoadSegment.
    */
    proto::RoadSegment toProtoSegment() const;

    /**
     * The id of a branch or a switch in RoadSet.cfg. Whether it's a branch or a swith
     * depends on railPathType().
     */
    int roadSetEntityId() const { return m_roadSetEntityId; }

    RailPathType railPathType() const { return m_railPathType; }

    bool isBranch() const { return railPathTypeIsBranch(m_railPathType); }

    bool isSwitch() const { return railPathTypeIsSwitch(m_railPathType); }

    bool isCross() const { return railPathTypeIsCross(m_railPathType); }

    /**
     * To be used for debugging purposes only.
     */
    std::string toString() const;

    size_t hashValue() const noexcept;

    friend bool operator==(RailPathId const& lhs, RailPathId const& rhs) noexcept
    {
        return lhs.toTuple() == rhs.toTuple();
    }

    friend bool operator!=(RailPathId const& lhs, RailPathId const& rhs) noexcept
    {
        return lhs.toTuple() != rhs.toTuple();
    }

    friend bool operator<(RailPathId const& lhs, RailPathId const& rhs) noexcept
    {
        return lhs.toTuple() < rhs.toTuple();
    }

    friend bool operator>(RailPathId const& lhs, RailPathId const& rhs) noexcept
    {
        return lhs.toTuple() > rhs.toTuple();
    }

private:
    std::tuple<int, RailPathType> toTuple() const noexcept
    {
        return std::tuple<int, RailPathType>(m_roadSetEntityId, m_railPathType);
    }

    int m_roadSetEntityId;
    RailPathType m_railPathType;
};


namespace std
{

template<> struct hash<RailPathId>
{
    using argument_type = RailPathId;
    using result_type = size_t;

    result_type operator()(argument_type const& railPathId) const noexcept
    {
        return railPathId.hashValue();
    }
};

} // namespace std
