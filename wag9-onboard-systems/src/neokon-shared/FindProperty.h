#pragma once

#include <UnigineNode.h>
#include <UnigineProperties.h>

/**
 * If the given node has a property with the given name, returns that property.
 * Otherwise, returns a null pointer.
 */
Unigine::PropertyPtr findProperty(Unigine::NodePtr const& node, char const* name);
