#include "ElectrificationNodeConnectorsData.h"

#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"

#include <UnigineNodes.h>
#include <UnigineWorld.h>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace
{
using namespace Unigine;
using namespace electrification;

std::unordered_map<std::string, ConnectorsMap> s_nodeConnectors;

const std::unordered_set<std::string> s_connectorNames = {POLE_CONNECTOR_ID_NT,
    POLE_CONNECTOR_ID_FT, POLE_CONNECTOR_ID_BFT, POLE_CONNECTOR_ID_TFT, POLE_CONNECTOR_ID_CROSS_INS,
    POLE_CONNECTOR_ID_KP_2WS_L, POLE_CONNECTOR_ID_KP_2WS_R, POLE_CONNECTOR_ID_KP_1WS_L,
    POLE_CONNECTOR_ID_KP_1WS_R, POLE_CONNECTOR_ID_KP_2WS, POLE_CONNECTOR_ID_KP_1WS,
    POLE_CONNECTOR_ID_KP_2WA, POLE_CONNECTOR_ID_KP_1WA, POLE_CONNECTOR_ID_KP_2WN,
    POLE_CONNECTOR_ID_KP_1WN, POLE_CONNECTOR_ID_KP_2WC, POLE_CONNECTOR_ID_KP_1WC,
    INSULATOR_CONNECTOR_NT_IN, INSULATOR_CONNECTOR_NT_OUT, INSULATOR_CONNECTOR_KP_1WS_IN,
    INSULATOR_CONNECTOR_KP_1WS_OUT, INSULATOR_CONNECTOR_KP_2WS_IN, INSULATOR_CONNECTOR_KP_2WS_OUT,
    INSULATOR_CONNECTOR_KP_2WS_A_IN, INSULATOR_CONNECTOR_KP_2WS_A_OUT,
    INSULATOR_CONNECTOR_KP_2WS_B_IN, INSULATOR_CONNECTOR_KP_2WS_B_OUT,
    ATTACHABLE_CONNECTOR_CROSS_WIRE_1, ATTACHABLE_CONNECTOR_CROSS_WIRE_2, CROSSBAR_CONNECTOR_LEFT,
    CROSSBAR_CONNECTOR_RIGHT, SUSPENSION_CONNECTOR_SPN, ATTACHABLE_CONNECTOR_DROPPER};


} // namespace

namespace electrification
{

void
fillNodeConnectorsData(std::string const& nodePath)
{
    if (nodePath.empty() || s_nodeConnectors.count(nodePath) != 0)
    {
        return;
    }

    auto& nodeConnectorsMapIt = s_nodeConnectors.find(nodePath);
    if (nodeConnectorsMapIt == s_nodeConnectors.end())
    {
        auto emplaceResult = s_nodeConnectors.emplace(nodePath, ConnectorsMap{});
        if (emplaceResult.second)
        {
            nodeConnectorsMapIt = emplaceResult.first;
        }
        else
        {
            return;
        }
    }

    ConnectorsMap& nodeConnectorsMap = nodeConnectorsMapIt->second;

    std::function<void(NodePtr const&, Math::Mat4 const&)> findConnectorNodes;
    findConnectorNodes = [&findConnectorNodes, &nodeConnectorsMap](NodePtr const& parentNode,
                             Math::Mat4 const& rootRotation)
    {
        if (!parentNode)
        {
            return;
        }

        if (parentNode->getType() == Node::NODE_REFERENCE)
        {
            auto referrenceNode = checked_ptr_cast<NodeReference>(parentNode);
            NodePtr node = referrenceNode->getReference();
            Math::Mat4 const rotate = Math::Mat4(Math::rotate(node->getRotation()));
            findConnectorNodes(node, rootRotation * Math::inverse(rotate));
        }

        int numChildren = parentNode->getNumChildren();
        for (int i = 0; i < numChildren; i++)
        {
            NodePtr childNode = parentNode->getChild(i);
            std::string childName = childNode->getName();

            auto prefixSearch = [&childName](std::string const& item)
            {
                return childName.find(item) != std::string::npos;
            };

            if (std::find_if(s_connectorNames.begin(), s_connectorNames.end(), prefixSearch) !=
                s_connectorNames.end())
            {
                Math::Mat4 transform = rootRotation * childNode->getWorldTransform();
                nodeConnectorsMap.emplace(childName, transform);
            }

            if (childNode->getNumChildren())
            {
                findConnectorNodes(childNode, rootRotation);
            }
        }
    };

    NodePtr sourceNode = World::loadNode(nodePath.c_str());
    if (!sourceNode)
    {
        Log::error("Couldn't load pole node \"%s\"", nodePath.c_str());
        return;
    }

    Math::Mat4 const rotate = Math::Mat4(Math::rotate(sourceNode->getRotation()));
    findConnectorNodes(sourceNode, Math::inverse(rotate));
    sourceNode.deleteLater();
}

ConnectorsMap
findNodeConnectorsData(std::string const& nodePath)
{
    auto foundIt = s_nodeConnectors.find(nodePath);
    if (foundIt != s_nodeConnectors.cend())
    {
        return foundIt->second;
    }

    return ConnectorsMap{};
}

} // namespace electrification
