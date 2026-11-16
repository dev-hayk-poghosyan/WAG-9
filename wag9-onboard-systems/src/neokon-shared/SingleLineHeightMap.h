#pragma once

#include "DirectedRoutePosition.h"
#include "ExtendedIntervalBound.h"
#include "HeightMapControlPoint.h"
#include "ProfilePosition.h"
#include "RailwayLine.h"

#include "SingleLineHeightMap.pb.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

class SingleLineHeightMap
{
public:
    /**
     * Loads a height map from a CSV file.
     *
     * This CSV file has to have at least two columns (additional ones are ignored):
     * -# The route position in kilometers (floating point number).
     * -# The height above the sea level (floating point number).
     *
     * The first row of the CSV file is assumed to contain field descriptions, and is ignored.
     *
     * @throw std::exception If anything goes wrong.
     */
    static SingleLineHeightMap fromCsvFile(
        std::filesystem::path const& csvFilePath,
        std::shared_ptr<RailwayLine const> line);

    /**
     * @throw std::exception If anything goes wrong.
     */
    static SingleLineHeightMap fromProto(
        proto::SingleLineHeightMap const& protoHeightMap,
        std::shared_ptr<RailwayLine const> line);

    proto::SingleLineHeightMap toProto() const;

    /**
     * Tries to resolve a directed route position into a profile position.
     *
     * In case of failure due to insufficient data being present (empty height map),
     * std::nullopt is returned.
     *
     * @throw std::exception If directedRoutePos belongs to the wrong railway line.
     *
     * @see RoutePosition::lineIndex()
     */
    std::optional<ProfilePosition> resolvePosition(
        DirectedRoutePosition const& directedRoutePos) const;

    /**
     * If there are control points between the given bounds (subject to bound types),
     * those controls points are reported through @p sink, in the order of them
     * being encountered when moving from @p lowerBound to @p upperBound.
     *
     * @throw std::exception If @p lowerBound or @p upperBound belong to the wrong
     *        railway line.
     *
     * @see RoutePosition::lineIndex()
     */
    void listControlPointsBetween(
        RoutePosition const& lowerBound, RoutePosition const& upperBound,
        ExtendedIntervalBound lowerBoundType, ExtendedIntervalBound upperBoundType,
        std::function<void(HeightMapControlPoint const&)> const& sink) const;

    double maxAbsSlopePerMille() const { return m_maxAbsSlopePerMille; }

private:
    struct Less;
    struct More;
    struct SlopeStats;

    SingleLineHeightMap(
        std::shared_ptr<RailwayLine const>&& line,
        std::vector<HeightMapControlPoint>&& orderedControlPoints,
        SlopeStats const& slopeStats);

    static void appendControlPoint(
        std::vector<HeightMapControlPoint>& orderedControlPoints,
        double positionM, double heightM);

    static SlopeStats calcSlopesAtControlPoints(
        std::vector<HeightMapControlPoint>& orderedControlPoints);

    template<typename Iter, typename Comparator>
    static void listControlPointsBetweenImpl(
        double lowerBoundPositionM, double upperBoundPositionM,
        ExtendedIntervalBound lowerBoundType, ExtendedIntervalBound upperBoundType,
        Iter sequenceBegin, Iter sequenceEnd, Comparator comp,
        std::function<void(HeightMapControlPoint const&)> const& sink);

    std::shared_ptr<RailwayLine const> m_line;

    /**
     * Poles ordered by increasing routePositionKM, which can't repeat or go down.
     */
    std::vector<HeightMapControlPoint> m_orderedControlPoints;

    double m_maxAbsSlopePerMille = 0.0;
};
