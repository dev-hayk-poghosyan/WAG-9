#pragma once

#include "neokon-shared/AutoBlockingSignalPropagation.h"
#include "neokon-shared/IntervalPropertyMap.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_hash.hpp>

#include <filesystem>
#include <unordered_map>

/**
 * If the specified file exists, loads the auto blocking signal propagation map.
 * If the file is missing, returns an empty map.
 *
 * @throw std::exception If anything else goes wrong.
 */
std::unordered_map<boost::uuids::uuid, IntervalPropertyMap<AutoBlockingSignalPropagation>>
    loadAutoBlockingSignalPropagationMap(std::filesystem::path const& propagationMapJsonFile);
