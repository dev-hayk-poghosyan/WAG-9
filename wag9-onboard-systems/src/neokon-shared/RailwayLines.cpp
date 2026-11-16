#include "RailwayLines.h"

#include "JsonStreamToProtoMessage.h"
#include "ProtoMessageToJsonStream.h"

#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

RailwayLines
RailwayLines::loadFromJsonFile(std::filesystem::path const& railwayLinesFilePath)
{
    std::ifstream strm(railwayLinesFilePath);
    if (!strm)
    {
        return createOldStyleSingleLineConfig();
    }

    proto::RailwayLines protoLines;
    jsonStreamToProtoMessage(strm, protoLines);

    std::vector<std::shared_ptr<RailwayLine>> lines;
    lines.reserve(protoLines.lines_size());
    
    for (proto::RailwayLines::Line const& protoLine : protoLines.lines())
    {
        auto line = std::make_shared<RailwayLine>(
            lines.size(), protoLine.name(),
            parseLineDistanceUnit(protoLine.distance_unit(), railwayLinesFilePath),
            parseRoutePositionFormat(protoLine.route_position_format(), railwayLinesFilePath),
            loadHeightMapFileRecord(protoLine.height_map_file(), railwayLinesFilePath),
            protoLine.id(), protoLine.color());

        lines.push_back(std::move(line));
    }

    if (lines.empty())
    {
        throw std::logic_error(str(boost::format(
            "No railway lines loaded from %1%") % railwayLinesFilePath.u8string()));
    }

    return RailwayLines(std::move(lines));
}

void RailwayLines::saveToJsonFile(std::filesystem::path const& railwayLinesFilePath) const
{
    std::ofstream strm(railwayLinesFilePath);
    if (!strm)
    {
        throw std::runtime_error(str(boost::format(
            "Unable to save railway lines to %1%") % railwayLinesFilePath.u8string()));
    }

    proto::RailwayLines protoLines;

    for (const auto& line : m_lines)
    {
        *protoLines.add_lines() = line->toProto();
    }

    protoMessageToJsonStream(protoLines, strm);
}

RailwayLines
RailwayLines::createOldStyleSingleLineConfig()
{
    auto line = std::make_shared<RailwayLine>(
        0, "<unnamed>", DistanceUnit::km(), RoutePositionFormat::KmPk,
        RailwayLine::HeightMapFile("route/Profile.csv", DistanceUnit::km(), DistanceUnit::m()),
        std::string(), std::string());

    return RailwayLines({ std::move(line) });
}

std::unordered_map<std::string, unsigned> RailwayLines::createIndexByIdRepository() const
{
    std::unordered_map<std::string, unsigned> lineIndexById;
    lineIndexById.reserve(m_lines.size());

    for (unsigned lineIdx = 0; lineIdx < m_lines.size(); ++lineIdx)
    {
        if (!m_lines[lineIdx]->id().empty())
        {
            lineIndexById[m_lines[lineIdx]->id()] = lineIdx;
        }
    }

    return lineIndexById;
}

RailwayLines::RailwayLines(std::vector<std::shared_ptr<RailwayLine>>&& lines)
    : m_lines(std::move(lines))
{
}

DistanceUnit
RailwayLines::parseLineDistanceUnit(
    std::string const& unitName, std::filesystem::path const& filePath)
{
    if (unitName == "km")
    {
        return DistanceUnit::km();
    }
    else if (unitName == "mi")
    {
        return DistanceUnit::mi();
    }

    throw std::logic_error(str(boost::format(
        "[%1%] Unsupported line distance unit \"%2%\". Only \"km\" and \"mi\" are supported")
        % filePath.u8string() % unitName));
}

RoutePositionFormat
RailwayLines::parseRoutePositionFormat(
    std::string const& formatName, std::filesystem::path const& railwayLinesFilePath)
{
    if (formatName == "KmPk")
    {
        return RoutePositionFormat::KmPk;
    }
    else if (formatName == "MilePoint")
    {
        return RoutePositionFormat::MilePoint;
    }

    throw std::logic_error(str(boost::format(
        "[%1%] Unsupported line distance format \"%2%\". "
        "Only \"KmPk\" and \"MilePoint\" are supported")
        % railwayLinesFilePath.u8string() % formatName));
}

DistanceUnit
RailwayLines::parseHeightMapPositionUnit(
    std::string const& unitName, std::filesystem::path const& railwayLinesFilePath)
{
    if (unitName == "km")
    {
        return DistanceUnit::km();
    }
    else if (unitName == "mi")
    {
        return DistanceUnit::mi();
    }

    throw std::logic_error(str(boost::format(
        "[%1%] Unsupported height map position unit \"%2%\". Only \"km\" and \"mi\" are supported")
        % railwayLinesFilePath.u8string() % unitName));
}

DistanceUnit
RailwayLines::parseHeightMapHeightUnit(
    std::string const& unitName, std::filesystem::path const& railwayLinesFilePath)
{
    if (unitName == "m")
    {
        return DistanceUnit::m();
    }
    else if (unitName == "ft")
    {
        return DistanceUnit::ft();
    }

    throw std::logic_error(str(boost::format(
        "[%1%] Unsupported height map height unit \"%2%\". Only \"m\" and \"ft\" are supported")
        % railwayLinesFilePath.u8string() % unitName));
}

RailwayLine::HeightMapFile
RailwayLines::loadHeightMapFileRecord(
    proto::RailwayLines::Line::HeightMapFile const& protoHeightMapFile,
    std::filesystem::path const& railwayLinesFilePath)
{
    return RailwayLine::HeightMapFile(
        protoHeightMapFile.world_relative_file_path(),
        parseHeightMapPositionUnit(protoHeightMapFile.position_unit(), railwayLinesFilePath),
        parseHeightMapHeightUnit(protoHeightMapFile.height_unit(), railwayLinesFilePath));
}
