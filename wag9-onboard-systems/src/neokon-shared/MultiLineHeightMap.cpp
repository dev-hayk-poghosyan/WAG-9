#include "MultiLineHeightMap.h"

#include "RailwayLines.h"

#include <algorithm>
#include <cstddef>
#include <utility>

MultiLineHeightMap
MultiLineHeightMap::loadForLines(
    std::filesystem::path const& worldDir, RailwayLines const& lines)
{
    std::vector<SingleLineHeightMap> perLineHeightMaps;
    perLineHeightMaps.reserve(lines.lines().size());

    for (std::shared_ptr<RailwayLine const> const& line : lines.lines())
    {
        perLineHeightMaps.push_back(
            SingleLineHeightMap::fromCsvFile(
                worldDir / line->heightMapFile().worldRelativeFilePath(), line));
    }

    return MultiLineHeightMap(std::move(perLineHeightMaps));
}

MultiLineHeightMap
MultiLineHeightMap::fromProto(
    proto::MultiLineHeightMap const& protoHeightMap, RailwayLines const& lines)
{
    if (lines.lines().size() != protoHeightMap.per_line_height_maps_size())
    {
        throw std::logic_error(
            "MutiLineHeightMap::fromProto(): disagreement over the number of railway lines");
    }

    unsigned const numLines = lines.lines().size();

    std::vector<SingleLineHeightMap> perLineHeightMaps;
    perLineHeightMaps.reserve(numLines);

    for (unsigned lineIdx = 0; lineIdx < numLines; ++lineIdx)
    {
        perLineHeightMaps.push_back(SingleLineHeightMap::fromProto(
            protoHeightMap.per_line_height_maps()[lineIdx], lines.lines()[lineIdx]));
    }

    return MultiLineHeightMap(std::move(perLineHeightMaps));
}

proto::MultiLineHeightMap
MultiLineHeightMap::toProto() const
{
    proto::MultiLineHeightMap protoHeightMap;

    for (SingleLineHeightMap const& singleLineHeightMap : m_perLineHeightMaps)
    {
        *protoHeightMap.add_per_line_height_maps() = singleLineHeightMap.toProto();
    }

    return protoHeightMap;
}

std::optional<ProfilePosition>
MultiLineHeightMap::resolvePosition(DirectedRoutePosition const& directedRoutePos) const
{
    size_t const lineIndex = directedRoutePos.routePosition().lineIndex();
    if (lineIndex >= m_perLineHeightMaps.size())
    {
        // No such line.
        return std::nullopt;
    }

    return m_perLineHeightMaps[lineIndex].resolvePosition(directedRoutePos);
}

void
MultiLineHeightMap::listControlPointsBetween(
    RoutePosition const& lowerBound, RoutePosition const& upperBound,
    ExtendedIntervalBound lowerBoundType, ExtendedIntervalBound upperBoundType,
    std::function<void(HeightMapControlPoint const&)> const& sink) const
{
    if (lowerBound.lineIndex() != upperBound.lineIndex())
    {
        throw std::logic_error(
            "MultiLineHeightMap::listControlPointsBetween():"
            " positions belonging to different railway lines passed");
    }

    unsigned const lineIdx = lowerBound.lineIndex();
    if (lineIdx >= m_perLineHeightMaps.size())
    {
        throw std::logic_error(
            "MultiLineHeightMap::listControlPointsBetween(): invalid railway line index");
    }

    m_perLineHeightMaps[lineIdx].listControlPointsBetween(
        lowerBound, upperBound, lowerBoundType, upperBoundType, sink);
}

MultiLineHeightMap::MultiLineHeightMap(std::vector<SingleLineHeightMap>&& perLineHeightMaps)
    : m_perLineHeightMaps(std::move(perLineHeightMaps))
{
    for (SingleLineHeightMap const& singleLineHeightMap : m_perLineHeightMaps)
    {
        m_maxAbsSlopePerMille = std::max(
            m_maxAbsSlopePerMille, singleLineHeightMap.maxAbsSlopePerMille());
    }
}
