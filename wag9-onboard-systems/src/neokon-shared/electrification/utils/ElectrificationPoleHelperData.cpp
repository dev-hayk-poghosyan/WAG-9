#include "ElectrificationPoleHelperData.h"

#include "neokon-shared/electrification/CatenaryGenerationConstants.h"

#include <functional>
#include <mutex>
#include <sstream>
#include <unordered_set>
#include <UnigineNodes.h>
#include <UnigineWorld.h>

namespace
{
using namespace Unigine;
using namespace electrification;

std::unordered_map<std::string, PoleSourceNodeHelperData> s_poleSourceNodeHelpers;
static std::mutex s_poleSourceNodeHelpersMutex;

const std::list<const char*> s_checkedConnectorNames = {POLE_CONNECTOR_ID_NT,
    POLE_CONNECTOR_ID_KP_2WS_L, POLE_CONNECTOR_ID_KP_2WS_R, POLE_CONNECTOR_ID_KP_1WS_L,
    POLE_CONNECTOR_ID_KP_1WS_R, POLE_CONNECTOR_ID_KP_2WS, POLE_CONNECTOR_ID_KP_1WS,
    POLE_CONNECTOR_ID_KP_2WA, POLE_CONNECTOR_ID_KP_1WA, POLE_CONNECTOR_ID_KP_2WN,
    POLE_CONNECTOR_ID_KP_1WN, POLE_CONNECTOR_ID_KP_2WC, POLE_CONNECTOR_ID_KP_1WC};

const std::unordered_map<std::string, std::string> s_kpToNtCastTable = {
    {POLE_CONNECTOR_ID_KP_1WS_L, POLE_CONNECTOR_ID_NT_WS_L},
    {POLE_CONNECTOR_ID_KP_1WS_R, POLE_CONNECTOR_ID_NT_WS_R},
    {POLE_CONNECTOR_ID_KP_2WS_L, POLE_CONNECTOR_ID_NT_WS_L},
    {POLE_CONNECTOR_ID_KP_2WS_R, POLE_CONNECTOR_ID_NT_WS_R},
    {POLE_CONNECTOR_ID_KP_1WS, POLE_CONNECTOR_ID_NT_WS},
    {POLE_CONNECTOR_ID_KP_1WA, POLE_CONNECTOR_ID_NT_WA},
    {POLE_CONNECTOR_ID_KP_1WN, POLE_CONNECTOR_ID_NT_WN},
    {POLE_CONNECTOR_ID_KP_1WC, POLE_CONNECTOR_ID_NT_WC},
    {POLE_CONNECTOR_ID_KP_2WS, POLE_CONNECTOR_ID_NT_WS},
    {POLE_CONNECTOR_ID_KP_2WA, POLE_CONNECTOR_ID_NT_WA},
    {POLE_CONNECTOR_ID_KP_2WN, POLE_CONNECTOR_ID_NT_WN},
    {POLE_CONNECTOR_ID_KP_2WC, POLE_CONNECTOR_ID_NT_WC}};

const std::string k_farConnectorSuffix = "_a";
const std::string k_nearConnectorSuffix = "_b";
const std::string k_tensioningMountPoleNodePrefix = "pole_";

void
correctZMistake(PoleCantileverConnectors& connectors)
{
    if (!connectors.wireFar && !connectors.wireNear)
    {
        return;
    }

    double cableZ = connectors.cable.getTranslate().z;
    double wireFarZ = connectors.wireFar ? connectors.wireFar->getTranslate().z : (cableZ - 1.0);
    double wireNearZ = connectors.wireNear ? connectors.wireNear->getTranslate().z : (cableZ - 1.0);

    if (connectors.wireFar && cableZ < wireFarZ)
    {
        std::swap(connectors.cable, *connectors.wireFar);
    }

    if (connectors.wireNear && cableZ < wireNearZ)
    {
        std::swap(connectors.cable, *connectors.wireNear);
    }
}

void
groupConnectorsByCantilevers(PoleSourceNodeHelperData& helperData)
{
    auto isNTConnector = [](std::string const& connectorName) -> bool
    {
        return connectorName.find("nt_") != std::string::npos;
    };

    auto findAllKPConnectorsForNT =
        [](std::string const& ntConnectorName,
            std::unordered_map<std::string, Unigine::Math::Mat4> const& connectorsByNames)
        -> PoleCantileverConnectors
    {
        PoleCantileverConnectors result;
        std::string prefix = ntConnectorName.substr(3); // rest chars after "nt_"

        for (auto const& [name, transform] : connectorsByNames)
        {
            if (name == ntConnectorName)
            {
                result.cable = transform;
                continue;
            }

            if (name.find(prefix) != std::string::npos)
            {
                if (name.find(k_nearConnectorSuffix) != std::string::npos)
                {
                    result.wireFar = transform;
                }
                else if (name.find(k_farConnectorSuffix) != std::string::npos)
                {
                    result.wireNear = transform;
                }
                else
                {
                    result.wireNear = transform;
                }
            }
        } // for (auto const& [name, transform] : connectorsByNames)

        return result;
    };

    for (auto const& [name, transform] : helperData.connectorsByNames)
    {
        if (!isNTConnector(name))
        {
            continue;
        }

        PoleCantileverConnectors& currentCantilever =
            findAllKPConnectorsForNT(name, helperData.connectorsByNames);
        correctZMistake(currentCantilever);
        helperData.connectorByCantilevers.emplace(name, std::move(currentCantilever));
    }
}

} // anonymous namespace

namespace electrification
{
const char* POLE_CONNECTOR_ID_NT = "nt";
const char* POLE_CONNECTOR_ID_KP_1WS = "kp_1ws";
const char* POLE_CONNECTOR_ID_KP_1WA = "kp_1wa";
const char* POLE_CONNECTOR_ID_KP_1WN = "kp_1wn";
const char* POLE_CONNECTOR_ID_KP_1WC = "kp_1wc";
const char* POLE_CONNECTOR_ID_KP_2WS = "kp_2ws";
const char* POLE_CONNECTOR_ID_KP_2WA = "kp_2wa";
const char* POLE_CONNECTOR_ID_KP_2WN = "kp_2wn";
const char* POLE_CONNECTOR_ID_KP_2WC = "kp_2wc";

const char* POLE_CONNECTOR_ID_NT_WS = "nt_ws";
const char* POLE_CONNECTOR_ID_NT_WA = "nt_wa";
const char* POLE_CONNECTOR_ID_NT_WN = "nt_wn";
const char* POLE_CONNECTOR_ID_NT_WC = "nt_wc";

const char* POLE_CONNECTOR_ID_KP_1WS_L = "kp_1ws_l";
const char* POLE_CONNECTOR_ID_KP_1WS_R = "kp_1ws_r";
const char* POLE_CONNECTOR_ID_KP_2WS_L = "kp_2ws_l";
const char* POLE_CONNECTOR_ID_KP_2WS_R = "kp_2ws_r";

const char* POLE_CONNECTOR_ID_NT_WS_L = "nt_ws_l";
const char* POLE_CONNECTOR_ID_NT_WS_R = "nt_ws_r";

const char* POLE_CONNECTOR_ID_FT = "ft0";
const char* POLE_CONNECTOR_ID_BFT = "bft0";
const char* POLE_CONNECTOR_ID_TFT = "tft0";

const char* POLE_CONNECTOR_ID_CROSS_INS = "cross_wire_insulator";
const char* SUSPENSION_CONNECTOR_SPN = "spn";

const char* INSULATOR_CONNECTOR_NT_IN = "nt_in";
const char* INSULATOR_CONNECTOR_NT_OUT = "nt_out";
const char* INSULATOR_CONNECTOR_KP_1WS_IN = "kp_1ws_in";
const char* INSULATOR_CONNECTOR_KP_1WS_OUT = "kp_1ws_out";
const char* INSULATOR_CONNECTOR_KP_2WS_IN = "kp_2ws_in";
const char* INSULATOR_CONNECTOR_KP_2WS_OUT = "kp_2ws_out";
const char* INSULATOR_CONNECTOR_KP_2WS_A_IN = "kp_2ws_a_in";
const char* INSULATOR_CONNECTOR_KP_2WS_A_OUT = "kp_2ws_a_out";
const char* INSULATOR_CONNECTOR_KP_2WS_B_IN = "kp_2ws_b_in";
const char* INSULATOR_CONNECTOR_KP_2WS_B_OUT = "kp_2ws_b_out";
const char* ATTACHABLE_CONNECTOR_CROSS_WIRE_1 = "cross_wire_1";
const char* ATTACHABLE_CONNECTOR_CROSS_WIRE_2 = "cross_wire_2";
const char* ATTACHABLE_CONNECTOR_DROPPER = "dropper";

const char* CROSSBAR_CONNECTOR_RIGHT = "pole1";
const char* CROSSBAR_CONNECTOR_LEFT = "pole2";

void
fillPoleSourceNodeHelperData(std::string const& nodePath)
{
    std::lock_guard<std::mutex> lock(s_poleSourceNodeHelpersMutex);

    if (nodePath.empty() || s_poleSourceNodeHelpers.count(nodePath) != 0)
    {
        return;
    }

    struct SiblingConnectors
    {
    public:
        std::unordered_map<std::string, Math::Mat4> connectorTransforms;
    };

    std::unordered_map<int, SiblingConnectors> siblingConnectors;
    std::optional<Math::Mat4> tensioningMountPoint;
    std::optional<Math::Mat4> crossbarMountPointLeft;
    std::optional<Math::Mat4> crossbarMountPointRight;

    auto addConnectorTransform =
        [&siblingConnectors](int key, std::string const& connectorName, Math::Mat4 const& transform)
    {
        if (siblingConnectors.count(key) == 0)
        {
            siblingConnectors.emplace(key, SiblingConnectors{});
        }

        SiblingConnectors& currentItem = siblingConnectors[key];
        assert(currentItem.connectorTransforms.count(connectorName) == 0);

        currentItem.connectorTransforms.emplace(connectorName, transform);
    };

    auto checkAndGetConnectorName = [](std::string const& name) -> std::optional<std::string>
    {
        for (const char* connectorNameSubstr : s_checkedConnectorNames)
        {
            if (name.find(connectorNameSubstr, 0) != std::string::npos)
            {
                if (name.find(k_farConnectorSuffix) != std::string::npos ||
                    name.find(k_nearConnectorSuffix) != std::string::npos)
                {
                    return name;
                }

                return std::string{connectorNameSubstr};
            }
        }

        return std::nullopt;
    };

    // Интелектуальный синтаксический разбор имен коннекторов Паши Политицкого
    // get KP connector name, for example:
    //  'kp_2ws_a' -> 'nt_ws'
    auto convertKPNameToNT = [](std::string const& kpName)
    {
        std::list<std::string> prefixes;
        std::size_t offset = 0;
        while (offset < kpName.size())
        {
            std::size_t pos = kpName.find_first_of('_', offset);
            if (pos != std::string::npos)
            {
                std::string prefix = kpName.substr(offset, pos - offset);
                prefixes.emplace_back(std::move(prefix));
                offset = pos + 1;
            }
            else
            {
                std::string prefix = kpName.substr(offset);
                prefixes.emplace_back(std::move(prefix));
                break;
            }
        }

        std::unordered_set<std::string> partsToSkip{"kp", "a", "b"};
        std::optional<std::string> lrPrefix;
        std::list<std::string> reorderedParts;

        for (std::string const& prefix : prefixes)
        {
            if (partsToSkip.count(prefix) != 0)
            {
                // discard
                continue;
            }

            if (std::isdigit(static_cast<unsigned char>(prefix[0])))
            {
                // 2ws, 1wa etc - discard digit
                reorderedParts.emplace_back(prefix.substr(1));
            }

            if (prefix == "l" || prefix == "r")
            {
                lrPrefix = prefix;
                continue;
            }
        }

        if (lrPrefix)
        {
            reorderedParts.push_back(*lrPrefix);
        }

        std::stringstream ss;
        ss << "nt_";
        while (reorderedParts.size() > 0)
        {
            ss << reorderedParts.front();
            reorderedParts.pop_front();

            if (reorderedParts.size() > 0)
            {
                ss << '_';
            }
        }

        std::string result = ss.str();

        return result;
    };

    std::function<void(NodePtr const&, Math::Mat4 const&)> findConnectorNodes;
    findConnectorNodes =
        [&findConnectorNodes, &checkAndGetConnectorName, &addConnectorTransform,
            &tensioningMountPoint, &crossbarMountPointLeft,
            &crossbarMountPointRight](NodePtr const& parentNode, Math::Mat4 const& rootRotation)
    {
        if (!parentNode)
        {
            return;
        }

        if (parentNode->getType() == Node::NODE_REFERENCE)
        {
            auto referrenceNode = checked_ptr_cast<NodeReference>(parentNode);
            NodePtr node = referrenceNode->getReference();
            findConnectorNodes(node, rootRotation);
        }

        int numChildren = parentNode->getNumChildren();
        for (int i = 0; i < numChildren; i++)
        {
            NodePtr childNode = parentNode->getChild(i);
            std::string childName = childNode->getName();

            if (auto connectorName = checkAndGetConnectorName(childName))
            {
                Math::Mat4 connectorBase = rootRotation * childNode->getWorldTransform();
                addConnectorTransform(parentNode->getID(), *connectorName, connectorBase);
            }
            else if (childName.find(k_tensioningMountPoleNodePrefix) == 0)
            {
                // each pole should have only one mount point
                if (!tensioningMountPoint)
                {
                    tensioningMountPoint = childNode->getWorldTransform();
                }
            }
            else if (childName == CROSSBAR_CONNECTOR_LEFT && !crossbarMountPointLeft)
            {
                crossbarMountPointLeft = childNode->getWorldTransform();
            }
            else if (childName == CROSSBAR_CONNECTOR_RIGHT && !crossbarMountPointRight)
            {
                crossbarMountPointRight = childNode->getWorldTransform();
            }

            findConnectorNodes(childNode, rootRotation);
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

    PoleSourceNodeHelperData helperData;
    helperData.tensioningMountPoint = tensioningMountPoint;
    helperData.crossbarMountPointLeft = crossbarMountPointLeft;
    helperData.crossbarMountPointRight = crossbarMountPointRight;

    std::unordered_map<std::string, std::optional<Math::Mat4>> ntConnectors;
    for (auto const& item : siblingConnectors)
    {
        std::optional<std::string> ntConnectorName;
        std::optional<Math::Mat4> ntConnectorTransform;

        SiblingConnectors const& connectorsSet = item.second;
        for (auto const& [name, transform] : connectorsSet.connectorTransforms)
        {
            if (name.find("kp_") != std::string::npos)
            {
                // contact wire
                std::string ntConnectorName = convertKPNameToNT(name);
                if (ntConnectors.count(ntConnectorName) == 0)
                {
                    ntConnectors.emplace(ntConnectorName, std::nullopt);
                }

                helperData.connectorsByNames.emplace(name, transform);
            }
            else
            {
                // support cable
                ntConnectorTransform = transform;
            }
        } // for (auto const& [name, transform] : connectorsSet.connectorTransforms)

        for (auto const& [name, transform] : connectorsSet.connectorTransforms)
        {
            auto findIt = ntConnectors.find(name);
            if (findIt != ntConnectors.end())
            {
                findIt->second = transform;
            }
        }

        if (ntConnectors.empty() && ntConnectorTransform)
        {
            // поддержка нод, состоящих только из троса
            if (connectorsSet.connectorTransforms.count(POLE_CONNECTOR_ID_NT) > 0)
            {
                ntConnectors.emplace(POLE_CONNECTOR_ID_NT_WC, ntConnectorTransform);
            }
            else if (connectorsSet.connectorTransforms.count(POLE_CONNECTOR_ID_FT) > 0)
            {
                ntConnectors.emplace(POLE_CONNECTOR_ID_FT, ntConnectorTransform);
            }
            else if (connectorsSet.connectorTransforms.count(POLE_CONNECTOR_ID_CROSS_INS) > 0)
            {
                ntConnectors.emplace(POLE_CONNECTOR_ID_CROSS_INS, ntConnectorTransform);
            }
        }

        assert(!ntConnectors.empty());
        assert(ntConnectorTransform);

        for (auto& [name, transform] : ntConnectors)
        {
            if (transform)
            {
                helperData.connectorsByNames.emplace(name, *transform);
            }
            else if (ntConnectorTransform)
            {
                helperData.connectorsByNames.emplace(name, *ntConnectorTransform);
            }
        }
    } // for (auto const& item : siblingConnectors)

    groupConnectorsByCantilevers(helperData);

    s_poleSourceNodeHelpers.emplace(nodePath, std::move(helperData));

    sourceNode.deleteLater();
}

std::optional<PoleSourceNodeHelperData>
findPoleNodeHelperData(std::string const& nodePath)
{
    std::lock_guard<std::mutex> lock(s_poleSourceNodeHelpersMutex);

    auto foundIt = s_poleSourceNodeHelpers.find(nodePath);
    if (foundIt != s_poleSourceNodeHelpers.cend())
    {
        return foundIt->second;
    }

    return std::nullopt;
}

} // namespace electrification
