#pragma once

#include "DistanceUnit.h"
#include "RailwayLine.h"
#include "RailwayLines.pb.h"
#include "RoutePositionFormat.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class RailwayLines
{
public:
    /**
     * Tries to load data from the specified file and create a RailwayLine instance from it.
     * If the file in question doesn't exist, creates a RailwayLines instances for legacy
     * worlds where we didn't have that file. If the file did exist but no lines were loaded
     * from it, an exception is thrown.
     *
     * @throw std::exception If anything goes wrong.
     */
    static RailwayLines loadFromJsonFile(std::filesystem::path const& railwayLinesFilePath);

    /**
     * Tries to save railway lines into specified file @p railwayLinesFilePath.
     * 
     * @throw std::exception on failure.
     */
    void saveToJsonFile(std::filesystem::path const& railwayLinesFilePath) const;

    std::vector<std::shared_ptr<RailwayLine>> const& lines() const { return m_lines; }
    std::vector<std::shared_ptr<RailwayLine>>& lines() { return m_lines; }

    /**
     * Railway line in fact has two identification property.
     * Index(an index of record in railway-line.json) and symbolic ID(proto::RailwayLines::Line::id).
     * Symbolic ID is more convenient way for big worlds for humans.
     * Numeric index is for backward compatibility.
     * This creates a map of numeric index on symbolic ID.
     */
    std::unordered_map<std::string, unsigned> createIndexByIdRepository() const;

private:
    RailwayLines(std::vector<std::shared_ptr<RailwayLine>>&& lines);

    static RailwayLines createOldStyleSingleLineConfig();

    static DistanceUnit parseLineDistanceUnit(
        std::string const& unitName, std::filesystem::path const& railwayLinesFilePath);

    static RoutePositionFormat parseRoutePositionFormat(
        std::string const& formatName, std::filesystem::path const& railwayLinesFilePath);

    static DistanceUnit parseHeightMapPositionUnit(
        std::string const& unitName, std::filesystem::path const& railwayLinesFilePath);

    static DistanceUnit parseHeightMapHeightUnit(
        std::string const& unitName, std::filesystem::path const& railwayLinesFilePath);

    static RailwayLine::HeightMapFile loadHeightMapFileRecord(
        proto::RailwayLines::Line::HeightMapFile const& protoHeightMapFile,
        std::filesystem::path const& railwayLinesFilePath);

    std::vector<std::shared_ptr<RailwayLine>> m_lines;
};
