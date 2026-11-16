#pragma once

#include <UnigineNode.h>
#include <UnigineProperties.h>

/**
 * If the given node has a property with the given name, or a property that directly
 * or indirectly inherits from it, returns that property. Otherwise, returns a null pointer.
 */
Unigine::PropertyPtr findPropertyOrSubClass(Unigine::NodePtr const& node, char const* name);
