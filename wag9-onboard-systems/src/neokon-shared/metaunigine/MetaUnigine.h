#pragma once

#include "MetaUnigineNodes.h"

namespace MetaUnigine
{

/**
 * @see Unigine::World::saveNode() function documentation.
 * 'nodeFilePath' is a path to *.node file in Unigine notation, absolute or 'data' directory relative
 * 'node' is a node to be saved
 * 'binary' is flag to save as binary xml
 * returns true of false in case of error
 */
bool saveNode(const char* nodeFilePath, const NodePtr& node, int binary = 0);
bool saveNodes(const char* nodeFilePath, const std::vector<NodePtr>& nodes, int binary = 0);

/**
 * @see Unigine::World::loadNode() function documentation.
 * 'nodeFilePath' is a path to *.node file in Unigine notation, absolute or 'data' directory relative
 * 'checkValid' is MetaUnigine extension to auto-validate loaded nodes
 * returns loaded node or empty pointer in case of error
 */
NodePtr loadNode(const char* nodeFilePath, bool checkValid = true);
std::vector<NodePtr> loadNodes(const char* nodeFilePath, bool checkValid = true);

/**
 * @see GTE::Core::getOrAddComponent() function.
 */
template <class TComponent>
TComponent* getOrAddComponent(NodePtr const& node)
{
    auto result = node->getComponent<TComponent>();
    if (!result)
    {
        result = node->addComponent<TComponent>();
    }

    return result;
};

}//namespace MetaUnigine
