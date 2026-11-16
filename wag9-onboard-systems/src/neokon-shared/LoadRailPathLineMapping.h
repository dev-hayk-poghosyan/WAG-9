#pragma once

#include "RailwayLines.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>

#include <filesystem>
#include <optional>
#include <unordered_map>

/**
 * If the specified file exists, loads the rail path UUID -> line index mapping.
 * If the file is missing, returns an empty map.
 *
 * If @p numLines is provided, this function will verify that the line indices
 * loaded from the specified JSON file are less then @p numLines.
 *
 * @throw std::exception If anything goes wrong.
 */
std::unordered_map<boost::uuids::uuid, unsigned> loadRailPathLineMapping(
    std::filesystem::path const& routeSetFilePath, const RailwayLines& railwayLines);
