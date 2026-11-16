#pragma once

#include <boost/uuid/uuid.hpp>

#include <string_view>

/**
 * @throws std::runtime_error if str has a wrong UUID format.
 * Valid formats:
 * {01234567-89ab-cdef-0123-456789abcdef}
 * 01234567-89ab-cdef-0123-456789abcdef
 * 0123456789abcdef0123456789abcdef
 */
boost::uuids::uuid uuidFromString(std::string_view str);
