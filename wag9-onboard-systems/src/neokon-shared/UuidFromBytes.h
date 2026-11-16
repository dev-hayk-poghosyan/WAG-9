#pragma once

#include <boost/uuid/uuid.hpp>

#include <string>

/**
 * @throws std::length_error if bytes.size() != 16.
 */
boost::uuids::uuid uuidFromBytes(std::string const& bytes);
