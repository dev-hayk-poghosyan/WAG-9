#pragma once

#include "RoutePositionFormat.h"
#include "RoutePosition.pb.h"

#include <memory>
#include <string>
#include <string_view>

class RailwayLines;

class RoutePosition
{
public:
    RoutePosition(
        unsigned lineIndex, int majorIntervalIdx,
        int minorIntervalIdx, int numMinorIntervals, double minorIntervalFraction);

    explicit RoutePosition(proto::RoutePosition const& protoPos);

    proto::RoutePosition toProto() const;

    /**
     * The index of the line in question in RailwayLines::lines().
     */
    unsigned lineIndex() const { return m_lineIndex; }

    /**
     * A major interval is a kilometer or a mile. It's further divided into minor intervals.
     *
     * Railway lines typically start at a station, and the station is assumed to be at distance 0
     * along the line. After 1 kilometer or 1 mile, there will be a major pole. That major pole
     * will mark the distance of 1 along the line. Everything in between is major interval 0.
     * In general, everything between the major poles with route position N and N+1 belongs to
     * the major interval N.
     *
     * @see RailwayLine::distanceUnit()
     */
    int majorIntervalIdx() const { return m_majorIntervalIdx; }

    /**
     * The sub-interval index within a large interval of majorIntervalIdx.
     * A sub-interval is either a picket or a quarter mile.
     * The index of the first is 0.
     */
    int minorIntervalIdx() const { return m_minorIntervalIdx; }

    /**
     * The number of minor intervals within this major interval.
     */
    int numMinorIntervals() const { return m_numMinorIntervals; }

    /**
     * Interpolation factor of location between minor poles.
     * Value in range: [0.0, 1.0].
     */
    double minorIntervalFraction() const { return m_minorIntervalFraction; }

    /**
     * Returns the fractional distance on route, in the same distance units as majorIntervalIdx().
     *
     * Example:
     * @code
     * majorIntervalIdx = 1;
     * minorIntervalIdx = 2;
     * numMinorIntervals = 10;
     * minorIntervalFraction = 0.5;
     * distanceOnRoute = 1 + (2 + 0.5) / 10;
     * @endcode
     *
     * @see RailwayLine::distanceUnit()
     */
    double distanceOnRoute() const;

    /**
     * Formats the route position according to the specified format.
     *
     * The returned string will be in UTF-8 encoding.
     */
    std::string toString(RoutePositionFormat format) const;

    /**
     * Same as toString(RoutePositionFormat), but the format is taken from the appropriate
     * railway line.
     *
     * In case m_lineIndex refers to a line missing from @p railwayLines, @p invalidLineReturn
     * is returned.
     */
    std::string toString(
        RailwayLines const& railwayLines,
        std::string_view invalidLineReturn = std::string_view()) const;

private:
    unsigned m_lineIndex = {};
    int m_majorIntervalIdx = {};
    int m_minorIntervalIdx = {};
    int m_numMinorIntervals = {};
    double m_minorIntervalFraction = {};
};

bool operator==(RoutePosition const& lhs, RoutePosition const& rhs);

bool operator!=(RoutePosition const& lhs, RoutePosition const& rhs);
