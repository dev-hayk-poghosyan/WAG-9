#pragma once

#include <UnigineNode.h>
#include <functional>

/**
 * Traverses node hierarchy calling sink for each node (or node that refers)
 * including root.
*/
void traverseNodeHierarchy(
    Unigine::NodePtr const& node,
    std::function<void(Unigine::NodePtr)> const& sink,
    bool followReferences = true, Unigine::NodePtr const& excludeHierarchy = {});

/**
 * Traverses node hierarchy calling sink for each node (or node that refers)
 * including root. if @p sink function returns true, the children or reference of the node
 * are processed, if false, the processing stops at the node's hierarchy branch.
 */
void traverseNodeHierarchyConditional(
    Unigine::NodePtr const& node,
    std::function<bool(Unigine::NodePtr)> const& sink,
    bool followReferences = true,
    Unigine::NodePtr const& excludeHierarchy = {});
