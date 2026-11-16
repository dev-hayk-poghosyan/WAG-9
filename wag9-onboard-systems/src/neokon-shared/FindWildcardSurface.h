#pragma once

#include <optional>
#include <UnigineObjects.h>

/**
 * Returns the id of the object's first surface whose name matches a pattern.
 *
 * The only special character in the pattern is '*', which matches any number of any characters.
 *
 * If no surfaces matching the pattern exist, boost::none is returned.
 */
std::optional<int> findWildcardSurface(
    Unigine::Object const& obj, char const* surfaceNamePattern, unsigned int startFrom = 0);
