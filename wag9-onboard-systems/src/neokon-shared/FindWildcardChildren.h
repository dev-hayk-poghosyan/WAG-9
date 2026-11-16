#pragma once

#include <UnigineNode.h>

#include <vector>

/**
 * Returns all the node's children whose names match a pattern.
 *
 * The only special character in the pattern is '*', which matches any number of any characters.
 */
std::vector<Unigine::NodePtr> findWildcardChildren(
    Unigine::NodePtr const& node, char const* childNamePattern);
