#pragma once

#include <UnigineNode.h>

/**
 * Returns the node's first child whose name matches a pattern.
 *
 * The only special character in the pattern is '*', which matches any number of any characters.
 *
 * If no children matching the pattern exist, a null Unigine::NodePtr is returned.
 */
Unigine::NodePtr findWildcardChild(
    Unigine::NodePtr const& node, char const* childNamePattern);
