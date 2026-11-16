#include "RailwayLine.h"

#include <cassert>

RailwayLine::RailwayLine(
    unsigned lineIndex, std::string_view name,
    DistanceUnit const& distanceUnit, RoutePositionFormat routePositionFormat,
    HeightMapFile const& heightMapFile, const std::string& id,
    const std::string& color)
    : m_lineIndex(lineIndex)
    , m_name(name)
    , m_distanceUnit(distanceUnit)
    , m_routePositionFormat(routePositionFormat)
    , m_heightMapFile(heightMapFile)
    , m_id(id)
    , m_color(color)
{
}

void
RailwayLine::setName(const std::string_view& name)
{
    m_name = name;
}

void
RailwayLine::setId(const std::string_view& id)
{
    m_id = id;
}

void
RailwayLine::setColor(const std::string_view& color)
{
    m_color = color;
}

proto::RailwayLines::Line
RailwayLine::toProto() const
{
    proto::RailwayLines::Line protoLine;

    protoLine.set_name(name());
    protoLine.set_id(id());
    protoLine.set_color(color());
    protoLine.set_distance_unit(m_distanceUnit.symbol());
    protoLine.set_route_position_format(m_routePositionFormat == RoutePositionFormat::KmPk
        ? "KmPk"
        : "MilePoint");

    proto::RailwayLines::Line::HeightMapFile hmFile;
    hmFile.set_world_relative_file_path(m_heightMapFile.worldRelativeFilePath().string());
    hmFile.set_position_unit(m_heightMapFile.positionUnit().symbol());
    hmFile.set_height_unit(m_heightMapFile.heightUnit().symbol());

    *protoLine.mutable_height_map_file() = hmFile;

    return protoLine;
}

std::string
RailwayLine::facingMajorPoleLabel(int majorPolePosition, bool facingIncreasingDistances) const
{
    if (m_routePositionFormat == RoutePositionFormat::KmPk)
    {
        return std::to_string(facingIncreasingDistances
            ? majorPolePosition
            : majorPolePosition + 1
        );
    }

    if (m_routePositionFormat == RoutePositionFormat::MilePoint)
    {
        return std::to_string(majorPolePosition);
    }

    assert(!"Unsupported route position format");
    return std::to_string(majorPolePosition);
}

/*==================================== HeightMapFile =================================*/

RailwayLine::HeightMapFile::HeightMapFile(
    std::filesystem::path const& worldRelativeFilePath,
    DistanceUnit const& positionUnit, DistanceUnit const& heightUnit)
    : m_worldRelativeFilePath(worldRelativeFilePath)
    , m_positionUnit(positionUnit)
    , m_heightUnit(heightUnit)
{
}
