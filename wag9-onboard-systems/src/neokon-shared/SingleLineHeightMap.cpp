#include "SingleLineHeightMap.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <parser.hpp> // part of "csv-parser" external package.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

struct SingleLineHeightMap::Less
{
    bool operator()(HeightMapControlPoint const& lhs, double rhs) const {
        return lhs.positionM < rhs;
    }

    bool operator()(double lhs, HeightMapControlPoint const& rhs) const {
        return lhs < rhs.positionM;
    }
};

struct SingleLineHeightMap::More
{
    bool operator()(HeightMapControlPoint const& lhs, double rhs) const {
        return lhs.positionM > rhs;
    }

    bool operator()(double lhs, HeightMapControlPoint const& rhs) const {
        return lhs > rhs.positionM;
    }
};

struct SingleLineHeightMap::SlopeStats
{
    double maxAbsSlopePerMille = 0.0;
};

SingleLineHeightMap
SingleLineHeightMap::fromCsvFile(
    std::filesystem::path const& csvFilePath,
    std::shared_ptr<RailwayLine const> line)
{
    assert(line);

    std::ifstream strm(csvFilePath);
    if (!strm)
    {
        throw std::runtime_error(str(boost::format(
            "Failed to open file: %s") % csvFilePath.generic_string()));
    }

    std::vector<HeightMapControlPoint> orderedControlPoints;

    aria::csv::CsvParser parser(strm);
    int lineInFile = 0;

    try
    {
        for (std::vector<std::string> const& row : parser)
        {
            ++lineInFile;
            if (lineInFile == 1)
            {
                // Skip the first line.
                continue;
            }

            double const distanceOnRoute = boost::lexical_cast<double>(row.at(0));
            if (distanceOnRoute < 0.0)
            {
                throw std::runtime_error(str(boost::format(
                    "[%s:%d] Negative distances are not allowed")
                    % csvFilePath.generic_string() % line));
            }
            double const positionM = line->distanceUnit().toMeters(distanceOnRoute);

            double const heightAboveSeaLevel = boost::lexical_cast<double>(row.at(1));
            double const heightM = line->heightMapFile().heightUnit().toMeters(heightAboveSeaLevel);

            appendControlPoint(orderedControlPoints, positionM, heightM);
        }
    }
    catch (boost::bad_lexical_cast const&)
    {
        throw std::runtime_error(str(boost::format(
            "[%s:%d] Failed to parse a number") % csvFilePath.u8string() % line));
    }
    catch (std::out_of_range const&)
    {
        throw std::runtime_error(str(boost::format(
            "[%s:%d] Required column doesn't exist") % csvFilePath.u8string() % line));
    }
    catch (std::exception const& e)
    {
        throw std::runtime_error(str(boost::format(
            "[%s:%d] %s") % csvFilePath.u8string() % line % e.what()));
    }

    SlopeStats const slopeStats = calcSlopesAtControlPoints(orderedControlPoints);
    return SingleLineHeightMap(std::move(line), std::move(orderedControlPoints), slopeStats);
}

SingleLineHeightMap
SingleLineHeightMap::fromProto(
    proto::SingleLineHeightMap const& protoHeightMap,
    std::shared_ptr<RailwayLine const> line)
{
    assert(line);

    std::vector<HeightMapControlPoint> orderedControlPoints;

    for (proto::SingleLineHeightMap::Entry const& entry : protoHeightMap.entries())
    {
        appendControlPoint(orderedControlPoints,
            entry.route_position_m(), entry.height_above_sea_level_m());
    }

    SlopeStats const slopeStats = calcSlopesAtControlPoints(orderedControlPoints);
    return SingleLineHeightMap(std::move(line), std::move(orderedControlPoints), slopeStats);
}

SingleLineHeightMap::SingleLineHeightMap(
    std::shared_ptr<RailwayLine const>&& line,
    std::vector<HeightMapControlPoint>&& orderedControlPoints,
    SlopeStats const& slopeStats)
    : m_line(std::move(line))
    , m_orderedControlPoints(std::move(orderedControlPoints))
    , m_maxAbsSlopePerMille(slopeStats.maxAbsSlopePerMille)
{
    assert(m_line);
}

proto::SingleLineHeightMap
SingleLineHeightMap::toProto() const
{
    proto::SingleLineHeightMap protoHeightMap;

    for (HeightMapControlPoint const& cp : m_orderedControlPoints)
    {
        proto::SingleLineHeightMap::Entry* entry = protoHeightMap.add_entries();
        entry->set_route_position_m(cp.positionM);
        entry->set_height_above_sea_level_m(cp.heightM);
    }

    return protoHeightMap;
}

std::optional<ProfilePosition>
SingleLineHeightMap::resolvePosition(DirectedRoutePosition const& directedRoutePos) const
{
    if (directedRoutePos.routePosition().lineIndex() != m_line->lineIndex())
    {
        throw std::logic_error(
            "SingleLineHeightMap: position beloning to a wrong railway line passed");
    }

    RoutePosition const& routePos = directedRoutePos.routePosition();
    double const targetPositionM = m_line->distanceUnit().toMeters(routePos.distanceOnRoute());

    auto const begin = m_orderedControlPoints.begin();
    auto const end = m_orderedControlPoints.end();

    // Find the first position that's greater or equal to the query.
    auto const next = std::lower_bound(begin, end, targetPositionM, Less());

    if (begin == end)
    {
        // No control points at all.
        return std::nullopt;
    }
    else if (next == end)
    {
        // directedRoutePos is past the end of the height map.
        return ProfilePosition(m_orderedControlPoints.back());
    }
    else if (next == begin)
    {
        // directedRoutePos is prior to the beginning of the height map.
        return ProfilePosition(m_orderedControlPoints.front());
    }

    auto const prev = std::prev(next);

    double const alpha = (targetPositionM - prev->positionM) /
        (next->positionM - prev->positionM);

    if (directedRoutePos.facingIncreasingKilometers())
    {
        return ProfilePosition(*prev, *next, alpha);
    }
    else
    {
        return ProfilePosition(*next, *prev, 1.0 - alpha);
    }
}

void
SingleLineHeightMap::listControlPointsBetween(
    RoutePosition const& lowerBound, RoutePosition const& upperBound,
    ExtendedIntervalBound lowerBoundType, ExtendedIntervalBound upperBoundType,
    std::function<void(HeightMapControlPoint const&)> const& sink) const
{
    unsigned const lineIndex = m_line->lineIndex();

    if (lowerBound.lineIndex() != lineIndex || upperBound.lineIndex() != lineIndex)
    {
        throw std::logic_error(
            "SingleLineHeightMap: position beloning to a wrong railway line passed");
    }

    double const lowerBoundPositionM =
        m_line->distanceUnit().toMeters(lowerBound.distanceOnRoute());

    double const upperBoundPositionM =
        m_line->distanceUnit().toMeters(upperBound.distanceOnRoute());

    if (lowerBoundPositionM <= upperBoundPositionM)
    {
        listControlPointsBetweenImpl(
            lowerBoundPositionM, upperBoundPositionM,
            lowerBoundType, upperBoundType,
            m_orderedControlPoints.begin(), m_orderedControlPoints.end(), Less(), sink);
    }
    else
    {
        listControlPointsBetweenImpl(
            lowerBoundPositionM, upperBoundPositionM,
            lowerBoundType, upperBoundType,
            m_orderedControlPoints.rbegin(), m_orderedControlPoints.rend(), More(), sink);
    }
}

void
SingleLineHeightMap::appendControlPoint(
    std::vector<HeightMapControlPoint>& orderedControlPoints,
    double positionM, double heightM)
{
    if (!orderedControlPoints.empty())
    {
        if (!(positionM > orderedControlPoints.back().positionM))
        {
            throw std::logic_error("Positions on the height map may only go up");
        }
    }

    orderedControlPoints.push_back(HeightMapControlPoint{ positionM, heightM, 0.0 });
}

SingleLineHeightMap::SlopeStats
SingleLineHeightMap::calcSlopesAtControlPoints(
    std::vector<HeightMapControlPoint>& orderedControlPoints)
{
    SlopeStats slopeStats{};

    size_t const numControlPoints = orderedControlPoints.size();
    if (numControlPoints > 0)
    {
        orderedControlPoints[0].slope = 0.0;

        for (size_t i = 1; i + 1 < numControlPoints; ++i)
        {
            HeightMapControlPoint& thisCP = orderedControlPoints[i];
            HeightMapControlPoint const& prevCP = orderedControlPoints[i - 1];
            HeightMapControlPoint const& nextCP = orderedControlPoints[i + 1];

            double const prevHeightChangeM = thisCP.heightM - prevCP.heightM;
            double const nextHeightChangeM = nextCP.heightM - thisCP.heightM;
            double const prevPositionChangeM = thisCP.positionM - prevCP.positionM;
            double const nextPositionChangeM = nextCP.positionM - thisCP.positionM;
            assert(prevPositionChangeM > 0.0 && nextPositionChangeM >= 0.0 &&
                "appendControlPoint() should have ensured that");

            // Here we rely on the height and distance units being the same.
            double const prevTangent = prevHeightChangeM / prevPositionChangeM;
            double const nextTangent = nextHeightChangeM / nextPositionChangeM;

            // We calculate the slope using the Finite Differencing scheme:
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Finite_difference
            thisCP.slope = 0.5 * (prevTangent + nextTangent);

            slopeStats.maxAbsSlopePerMille = std::max(
                slopeStats.maxAbsSlopePerMille, std::abs(thisCP.slope * 1000.0));
        }

        orderedControlPoints[numControlPoints - 1].slope = 0.0;
    }

    return slopeStats;
}

template<typename Iter, typename Comparator>
void
SingleLineHeightMap::listControlPointsBetweenImpl(
    double lowerBoundPositionM, double upperBoundPositionM,
    ExtendedIntervalBound lowerBoundType, ExtendedIntervalBound upperBoundType,
    Iter sequenceBegin, Iter sequenceEnd, Comparator comp,
    std::function<void(HeightMapControlPoint const&)> const& sink)
{
    auto getRangeBeginIter = [comp, lowerBoundPositionM, lowerBoundType](Iter begin, Iter end)
    {
        if (lowerBoundType == ExtendedIntervalBound::Inclusive)
        {
            // Find the first element that's greater or equal to the target.
            return std::lower_bound(begin, end, lowerBoundPositionM, comp);
        }
        else // Exclusive or ExtraInclusive
        {
            // Find the first element that's strictly greater than the target.
            auto it = std::upper_bound(begin, end, lowerBoundPositionM, comp);
            if (lowerBoundType == ExtendedIntervalBound::ExtraInclusive)
            {
                if (it != begin)
                {
                    --it;
                }
            }
            return it;
        }
    };

    // Note that the iterator returned from this function is one-past-the-end of the range.
    auto getRangeEndIter = [comp, upperBoundPositionM, upperBoundType](Iter begin, Iter end)
    {
        if (upperBoundType == ExtendedIntervalBound::Inclusive)
        {
            // Find the first element that's strictly greater than the target.
            return std::upper_bound(begin, end, upperBoundPositionM, comp);
        }
        else // Exclusive or ExtraInclusive
        {
            // Find the first element that's greater or equal to the target.
            auto it = std::lower_bound(begin, end, upperBoundPositionM, comp);
            if (upperBoundType == ExtendedIntervalBound::ExtraInclusive)
            {
                if (it != end)
                {
                    // We may be standing either at the target element, or if it's missing,
                    // at the following one. In either case, we have to include it in the range.
                    // Don't forget we are returning the one-past-the-end iterator.
                    ++it;
                }
            }
            return it;
        }
    };

    // [begin, end) is a semi-closed interval.
    auto const begin = getRangeBeginIter(sequenceBegin, sequenceEnd);
    auto const end = getRangeEndIter(begin, sequenceEnd);

    for (auto it = begin; it != end; ++it)
    {
        sink(*it);
    }
}
