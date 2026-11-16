#pragma once

#include "ExtendedIntervalBound.h"
#include "MultiLineHeightMap.pb.h"
#include "ProfilePosition.h"
#include "SingleLineHeightMap.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

class RailwayLines;
class DirectedRoutePosition;

class MultiLineHeightMap
{
public:
    /**
     * @throw std::exception If anything goes wrong.
     */
    static MultiLineHeightMap loadForLines(
        std::filesystem::path const& worldDir, RailwayLines const& lines);

    /**
     * @throw std::exception If anything goes wrong.
     */
    static MultiLineHeightMap fromProto(
        proto::MultiLineHeightMap const& protoHeightMap, RailwayLines const& lines);

    proto::MultiLineHeightMap toProto() const;

    std::optional<ProfilePosition> resolvePosition(
        DirectedRoutePosition const& directedRoutePos) const;

    /**
     * If there are control points between the given bounds (subject to bound types),
     * located on the same railway line, those controls points are reported through @p sink,
     * in the order of them being encountered when moving from @p lowerBound to @p upperBound.
     *
     * @throw std::exception If @p lowerBound and @p upperBound belong to
     *        different or invalid railway lines.
     *
     * @see RoutePosition::lineIndex()
     */
    void listControlPointsBetween(
        RoutePosition const& lowerBound, RoutePosition const& upperBound,
        ExtendedIntervalBound lowerBoundType, ExtendedIntervalBound upperBoundType,
        std::function<void(HeightMapControlPoint const&)> const& sink) const;

    double maxAbsSlopePerMille() const { return m_maxAbsSlopePerMille; }

private:
    MultiLineHeightMap(std::vector<SingleLineHeightMap>&& perLineHeightMaps);

    /**
     * Height maps for individual lines, with line id indexing this vector.
     */
    std::vector<SingleLineHeightMap> m_perLineHeightMaps;

    double m_maxAbsSlopePerMille = 0.0;
};
