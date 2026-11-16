#include "RoutePosition.h"

#include "RailwayLines.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

RoutePosition::RoutePosition(
    unsigned lineIndex, int majorIntervalIdx,
    int minorIntervalIdx, int numMinorIntervals, double minorIntervalFraction)
    : m_lineIndex(lineIndex)
    , m_majorIntervalIdx(majorIntervalIdx)
    , m_minorIntervalIdx(minorIntervalIdx)
    , m_numMinorIntervals(numMinorIntervals)
    , m_minorIntervalFraction(minorIntervalFraction)
{
}

RoutePosition::RoutePosition(
    proto::RoutePosition const& protoPos)
    : m_lineIndex(protoPos.line_idx())
    , m_majorIntervalIdx(protoPos.major_interval_idx())
    , m_minorIntervalIdx(protoPos.minor_interval_idx())
    , m_numMinorIntervals(protoPos.num_minor_intervals())
    , m_minorIntervalFraction(protoPos.minor_interval_fraction())
{
}

proto::RoutePosition
RoutePosition::toProto() const
{
    proto::RoutePosition protoPos;

    protoPos.set_line_idx(m_lineIndex);
    protoPos.set_major_interval_idx(m_majorIntervalIdx);
    protoPos.set_minor_interval_idx(m_minorIntervalIdx);
    protoPos.set_num_minor_intervals(m_numMinorIntervals);
    protoPos.set_minor_interval_fraction(m_minorIntervalFraction);

    return protoPos;
}

double
RoutePosition::distanceOnRoute() const
{
    return m_majorIntervalIdx +
        (m_minorIntervalIdx + m_minorIntervalFraction)
        / double(std::max<int>(1, m_numMinorIntervals));
}

std::string
RoutePosition::toString(RoutePositionFormat format) const
{
    std::ostringstream strm;

    switch (format)
    {
    case RoutePositionFormat::KmPk:
        // Note that both kilometers are piquets start from 1, while major and minor
        // interval indices start from zero.
        strm << (m_majorIntervalIdx + 1) << " км ПК" << (m_minorIntervalIdx + 1);
        break;
    case RoutePositionFormat::MilePoint:
        strm << std::fixed << std::setprecision(1) << distanceOnRoute() << " mi";
        break;
    }

    return strm.str();
}

std::string
RoutePosition::toString(
    RailwayLines const& railwayLines, std::string_view invalidLineReturn) const
{
    size_t const numLines = railwayLines.lines().size();
    if (m_lineIndex >= numLines)
    {
        return std::string(invalidLineReturn);
    }

    return toString(railwayLines.lines()[m_lineIndex]->routePositionFormat());
}

bool operator==(RoutePosition const& lhs, RoutePosition const& rhs)
{
    return lhs.lineIndex() == rhs.lineIndex() &&
        lhs.majorIntervalIdx() == rhs.majorIntervalIdx() &&
        lhs.minorIntervalIdx() == rhs.minorIntervalIdx() &&
        lhs.numMinorIntervals() == rhs.numMinorIntervals() &&
        lhs.minorIntervalFraction() == rhs.minorIntervalFraction();
}

bool operator!=(RoutePosition const& lhs, RoutePosition const& rhs)
{
    return !(lhs == rhs);
}
