#pragma once

#include <UnigineMathLib.h>

#include <string>
#include <optional>
#include <unordered_map>

namespace electrification
{

using ConnectorsMap = std::unordered_map<std::string, Unigine::Math::Mat4>;

/**
 * Does nothing if the node with path @p nodePath has already been loaded.
 *
 * Must be called in the main thread.
 */
void fillNodeConnectorsData(std::string const& nodePath);

ConnectorsMap findNodeConnectorsData(std::string const& nodePath);

} // namespace electrification
