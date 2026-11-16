#pragma once

#include <UnigineNode.h>

#include <string>
#include <vector>

/**
 * Takes a node and walks down the path of its descendants.
 *
 * A path in the node hierarchy is very much like a standard Unix path, for instance
 * "/" or "/a/b/c".
 *
 * One special case of a path component is the "@" symbol. It causes
 * the current node to be cast to Unigine::NodeReference and then dereferenced.
 * So, for instance "/a/@/b" means to take the root node's child "a", dereference it
 * and finally take the child "b" of the dereferenced node.
 *
 * Besides the "@" operator, path components can contain wildcards, like this: "/a*c/d*f"
 *
 * @note The leading slash is optional.
 *
 * Errors are reported through exceptions. If @p exceptionStringPrefix is specified,
 * it will get prepended to the exception's message.
 *
 * @throw std::exception If anything goes wrong.
 */
Unigine::NodePtr resolveNodePathOrThrow(
    Unigine::NodePtr const& rootNode, char const* path,
    char const* exceptionStringPrefix = nullptr);

/**
 * Just like resolveNodePathOrThrow(), but returns a null Unigine::NodePtr instead
 * of throwing an exception.
 */
Unigine::NodePtr tryResolveNodePath(
    Unigine::NodePtr const& rootNode, char const* path);

/**
 * Just like resolveNodePathOrThrow(), but takes many paths as a parameter.
 */
Unigine::NodePtr resolveNodePathsOrThrow(
    Unigine::NodePtr const& rootNode, std::vector<std::string> const& paths,
    char const* exceptionStringPrefix = nullptr);

/**
 * Just like tryResolveNodePath(), but takes many paths as a parameter.
 */
Unigine::NodePtr tryResolveNodePaths(
    Unigine::NodePtr const& rootNode, std::vector<std::string> const& paths);
