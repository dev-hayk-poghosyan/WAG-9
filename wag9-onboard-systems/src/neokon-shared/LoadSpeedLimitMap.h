#pragma once

#include "neokon-shared/SpeedLimitProperty.h"
#include "neokon-shared/IntervalPropertyMap.h"

#include <glm/glm.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>

#include <filesystem>
#include <unordered_map>
#include <optional>
#include <functional>

struct WorldSpeedLimitMap
{
    std::optional<SpeedLimitProperty> defaultSpeedLimits;
    std::unordered_map<boost::uuids::uuid, IntervalPropertyMap<SpeedLimitProperty>> speedMap;
};

struct WorlSpeedLimitMapResolveData
{
    boost::uuids::uuid railPathUuid;
    double distanceAlongPath{};
};

/**
 * A resolver of a point on the rails for the specified point in the world coordinates.
 */
using PointOnRoadResolver = std::function<std::optional<WorlSpeedLimitMapResolveData>(
    glm::dvec3 const& /*pointOnMap*/, double/* maxDistance*/)>;

/**
 * If the specified file exists, loads the speed limit map.
 * If the file is missing, returns an empty map.
 *
 * @throw std::exception If anything else goes wrong.
 */ 
WorldSpeedLimitMap loadSpeedLimitMap(
    std::filesystem::path const& speedLimitMapJsonFile, PointOnRoadResolver const& resolver);
