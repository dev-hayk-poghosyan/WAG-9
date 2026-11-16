#pragma once

#include <boost/property_tree/ptree_fwd.hpp>

/**
 * Overrides all nodes from @p overrideTree in @p baseTree at the correspond paths.
 */
void overrideTree(boost::property_tree::ptree& baseTree,
    boost::property_tree::ptree const& overrideTree);
