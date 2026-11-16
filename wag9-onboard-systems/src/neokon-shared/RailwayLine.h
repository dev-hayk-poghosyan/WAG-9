#pragma once

#include "DistanceUnit.h"
#include "RailwayLines.pb.h"
#include "RoutePositionFormat.h"

#include <filesystem>
#include <string>
#include <string_view>

class RailwayLine
{
public:
    class HeightMapFile
    {
    public:
        HeightMapFile(
            std::filesystem::path const& worldRelativeFilePath,
            DistanceUnit const& positionUnit, DistanceUnit const& heightUnit);

        std::filesystem::path const& worldRelativeFilePath() const {
            return m_worldRelativeFilePath;
        }

        DistanceUnit const& positionUnit() const { return m_positionUnit; }

        DistanceUnit const& heightUnit() const { return m_heightUnit; }

    private:
        std::filesystem::path m_worldRelativeFilePath;
        DistanceUnit m_positionUnit;
        DistanceUnit m_heightUnit;
    };

    RailwayLine(unsigned lineIndex, std::string_view name,
        DistanceUnit const& distanceUnit, RoutePositionFormat routePositionFormat,
        HeightMapFile const& heightMapFile, const std::string& id,
        const std::string& color);

    unsigned lineIndex() const { return m_lineIndex; }

    std::string const& name() const { return m_name; }
    void setName(const std::string_view& name);

    DistanceUnit const& distanceUnit() const { return m_distanceUnit; }

    RoutePositionFormat routePositionFormat() const { return m_routePositionFormat; }

    HeightMapFile const& heightMapFile() const { return m_heightMapFile; }

    const std::string& id() const { return m_id; }
    void setId(const std::string_view& id);

    const std::string& color() const { return m_color; }
    void setColor(const std::string_view& color);

    proto::RailwayLines::Line toProto() const;

    /**
     * Returns the label on the major pole at a given kilometer or mile position, when facing
     * the specified direction.
     *
     * The label returned is in UTF-8 encoding, although it's usually just a number.
     *
     * @param majorPolePosition Corresponds to proto::MajorPole::route_position. See comments
     *        in MajorPole.proto.
     * @param facingIncreasingDistances This parameter is important, because major poles in
     *        interval-based systems have different labels on different sides of the pole.
     *        Such labels indicate the interval number (starting at 1) that ends at this pole.
     */
    std::string facingMajorPoleLabel(int majorPolePosition, bool facingIncreasingDistances) const;

private:
    unsigned m_lineIndex = 0;
    std::string m_name;
    DistanceUnit m_distanceUnit;
    RoutePositionFormat m_routePositionFormat = RoutePositionFormat::KmPk;
    HeightMapFile m_heightMapFile;
    std::string m_id;
    std::string m_color;
};
