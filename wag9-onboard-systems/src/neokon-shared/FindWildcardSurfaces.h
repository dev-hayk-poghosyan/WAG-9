#pragma once

#include <UnigineObjects.h>

#include <vector>
#include <string>

/**
 * Returns the IDs of the object's surfaces whose names match the provided pattern.
 *
 * The only special character in the pattern is '*', which matches any number of any characters.
 * List of surface names can be obtained in optional @p surfaceNames.
 */
std::vector<int> findWildcardSurfaces(
    Unigine::Object const& obj, char const* surfaceNamePattern,
    std::vector<std::string>* surfaceNames = nullptr);
