#include "DynamicElectrificationCatenaryAttachable.h"

#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/DynamicElectrificationPart.h"
#include "neokon-shared/electrification/utils/ElectrificationNodeConnectorsData.h"
#include "neokon-shared/TransformTurnaroundZ.h"

using namespace electrification;

using namespace Unigine;

namespace
{
/**
 * 0 - flipped == false, 1 - flipped == true.
 */
using FlippingConnectorPositionByType = std::array<ConnectorPosByType, 2>;

static std::unordered_map<std::string, FlippingConnectorPositionByType> s_insulatorConnectorsByPath;

// @note Must be called from the main thread.
ConnectorPosByType fillConnectorsInfoForInsulator(std::string const& nodePath, bool flipped)
{
    ConnectorPosByType connectors;

    auto assignPositionIfPossible = [&connectors](Math::Vec3 const& position,
        std::string const& searchString, const char* connectorName, AttachableConnectorType type)
    {
        if (searchString.find(connectorName) != std ::string::npos)
        {
            connectors.emplace(type, position);
        }
    };

    std::function<void(std::string const&)> findConnectorNodes;
    findConnectorNodes = [&connectors, &findConnectorNodes, &assignPositionIfPossible, flipped](
        std::string const& nodePath)
    {
        ConnectorsMap connectorsMap = findNodeConnectorsData(nodePath);
        for (auto& [connector, transform] : connectorsMap)
        {
            if (flipped)
            {
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_IN, AttachableConnectorType::NT_OUT);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_OUT, AttachableConnectorType::NT_IN);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    ATTACHABLE_CONNECTOR_CROSS_WIRE_1, AttachableConnectorType::NT_IN);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_IN, AttachableConnectorType::KP_NEAR_OUT);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_OUT, AttachableConnectorType::KP_NEAR_IN);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_1WS_IN, AttachableConnectorType::KP_NEAR_OUT);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_1WS_OUT, AttachableConnectorType::KP_NEAR_IN);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_IN, AttachableConnectorType::KP_NEAR_OUT);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_OUT, AttachableConnectorType::KP_NEAR_IN);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_A_IN, AttachableConnectorType::KP_NEAR_OUT);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_A_OUT, AttachableConnectorType::KP_NEAR_IN);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_B_IN, AttachableConnectorType::KP_FAR_OUT);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_B_OUT, AttachableConnectorType::KP_FAR_IN);
            }
            else
            {
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_IN, AttachableConnectorType::NT_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_OUT, AttachableConnectorType::NT_OUT);

                // TODO: здесь нужно учесть возможность добавления до четырёх тросов
                // через соответствия коннектор на ноде - коннектор на изоляторе
                assignPositionIfPossible(transform.getTranslate(), connector,
                    ATTACHABLE_CONNECTOR_CROSS_WIRE_2, AttachableConnectorType::NT_OUT);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_IN, AttachableConnectorType::KP_NEAR_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_NT_OUT, AttachableConnectorType::KP_NEAR_OUT);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_1WS_IN, AttachableConnectorType::KP_NEAR_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_1WS_OUT, AttachableConnectorType::KP_NEAR_OUT);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_IN, AttachableConnectorType::KP_NEAR_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_OUT, AttachableConnectorType::KP_NEAR_OUT);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_A_IN, AttachableConnectorType::KP_NEAR_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_A_OUT, AttachableConnectorType::KP_NEAR_OUT);

                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_B_IN, AttachableConnectorType::KP_FAR_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    INSULATOR_CONNECTOR_KP_2WS_B_OUT, AttachableConnectorType::KP_FAR_OUT);
            }

            if (connectorsMap.find(ATTACHABLE_CONNECTOR_CROSS_WIRE_1) != connectorsMap.end() ||
                connectorsMap.find(ATTACHABLE_CONNECTOR_CROSS_WIRE_2) != connectorsMap.end())
            {
                if (flipped)
                {
                    connectors.emplace(AttachableConnectorType::NT_OUT, Math::Vec3::ZERO);
                }
                else
                {
                    connectors.emplace(AttachableConnectorType::NT_IN, Math::Vec3::ZERO);
                }
            }
        }
    };

    if (nodePath.empty())
    {
        // TODO: maybe log error.
        return {};
    }

    NodePtr insulatorNode = World::loadNode(nodePath.c_str());
    if (!insulatorNode)
    {
        Unigine::Log::error("Couldn't load node \"%s\" for insulator\n", nodePath.c_str());
        return {};
    }
    findConnectorNodes(nodePath);
    insulatorNode.deleteLater();

    return connectors;
}

// @note Must be called from the main thread.
ConnectorPosByType
fillConnectorsInfoForDropperClamp(std::string const& nodePath, bool flipped)
{
    ConnectorPosByType connectors;

    auto assignPositionIfPossible = [&connectors](Math::Vec3 const& position,
                                        std::string const& searchString, const char* connectorName,
                                        AttachableConnectorType type)
    {
        if (searchString.find(connectorName) != std ::string::npos)
        {
            connectors.emplace(type, position);
        }
    };

    std::function<void(std::string const&)> findConnectorNodes;
    findConnectorNodes = [&connectors, &findConnectorNodes, &assignPositionIfPossible, flipped](
                             std::string const& nodePath)
    {
        ConnectorsMap connectorsMap = findNodeConnectorsData(nodePath);
        for (auto& [connector, transform] : connectorsMap)
        {
            assignPositionIfPossible(transform.getTranslate(), connector,
                ATTACHABLE_CONNECTOR_DROPPER, AttachableConnectorType::DROPPER);

            if (flipped)
            {
                assignPositionIfPossible(transform.getTranslate(), connector,
                    ATTACHABLE_CONNECTOR_CROSS_WIRE_1, AttachableConnectorType::NT_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    ATTACHABLE_CONNECTOR_CROSS_WIRE_1, AttachableConnectorType::NT_OUT);
            }
            else
            {
                assignPositionIfPossible(transform.getTranslate(), connector,
                    ATTACHABLE_CONNECTOR_CROSS_WIRE_2, AttachableConnectorType::NT_IN);
                assignPositionIfPossible(transform.getTranslate(), connector,
                    ATTACHABLE_CONNECTOR_CROSS_WIRE_2, AttachableConnectorType::NT_OUT);
            }
        }
    };

    if (nodePath.empty())
    {
        // TODO: maybe log error.
        return {};
    }

    NodePtr dropperClampNode = World::loadNode(nodePath.c_str());
    if (!dropperClampNode)
    {
        Unigine::Log::error("Couldn't load node \"%s\" for insulator\n", nodePath.c_str());
        return {};
    }
    findConnectorNodes(nodePath);
    dropperClampNode.deleteLater();

    return connectors;
}

} // anonymous namespace

/////////////////////////////////////////////////////////////////////////////////////////
// class MorticeAttachable
/////////////////////////////////////////////////////////////////////////////////////////
MorticeAttachable::MorticeAttachable(std::string nodeName, double distance, bool useDroppers, bool isFlipped)
    : m_nodeName(nodeName)
    , m_distance(distance)
    , m_useDroppers(useDroppers)
    , m_flipped(isFlipped)
{
    m_internalMeshTransform = DynamicElectrificationPart::getOrLoadInitialNodeTransform(m_nodeName)
                                  .value_or(Math::Mat4_identity);
}

std::optional<Math::Vec3>
MorticeAttachable::getConnectorPosition(AttachableConnectorType type) const
{
    auto findResult = m_connectors.find(type);
    if (findResult != m_connectors.end())
    {
        return findResult->second;
    }

    return std::nullopt;
}

Math::Mat4
MorticeAttachable::getWorldWithNoInitialTransform() const
{
    return m_worldWithNoInitialTransform;
}

void
MorticeAttachable::setWorldTransform(Math::Mat4 const& transform)
{
    m_worldTransform = transform * m_internalMeshTransform;
    m_worldWithNoInitialTransform = transform;

    if (m_flipped)
    {
        m_worldTransform = transformTurnaroundZ(m_worldTransform);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// class Insulator
/////////////////////////////////////////////////////////////////////////////////////////
Insulator::Insulator(std::string nodeName, double distance, bool useDroppers, bool isFlipped)
    : MorticeAttachable(nodeName, distance, useDroppers, isFlipped)
{
    fillConnectorsInfo();
}

void
Insulator::fillConnectorsInfo()
{
    const std::string path = nodeName();
    size_t const flippedIdx = isFlipped() ? 1 : 0;
    auto connectorsIter = s_insulatorConnectorsByPath.find(path);
    if (connectorsIter == s_insulatorConnectorsByPath.end())
    {
        auto iterPair = s_insulatorConnectorsByPath.emplace(path,
            FlippingConnectorPositionByType{fillConnectorsInfoForInsulator(path, /*flipped=*/false),
                fillConnectorsInfoForInsulator(path, /*flipped=*/true)});
        m_connectors = iterPair.first->second[flippedIdx];
    }
    else
    {
        m_connectors = connectorsIter->second[flippedIdx];
    }

    auto kpNearIn = getConnectorPosition(AttachableConnectorType::KP_NEAR_IN);
    auto kpNearOut = getConnectorPosition(AttachableConnectorType::KP_NEAR_OUT);
    auto kpFarIn = getConnectorPosition(AttachableConnectorType::KP_FAR_IN);
    auto kpFarOut = getConnectorPosition(AttachableConnectorType::KP_FAR_OUT);

    if (kpNearIn && kpFarIn)
    {
        Math::Vec3 position = *kpFarIn - *kpNearIn;
        position = *kpNearIn + position * 0.5;

        m_connectors.emplace(AttachableConnectorType::KP_DROPPER_IN, position);
    }
    else if (kpNearIn)
    {
        m_connectors.emplace(AttachableConnectorType::KP_DROPPER_IN, *kpNearIn);
    }

    if (kpNearOut && kpFarOut)
    {
        Math::Vec3 position = *kpFarOut - *kpNearOut;
        position = *kpNearOut + position * 0.5;

        m_connectors.emplace(AttachableConnectorType::KP_DROPPER_OUT, position);
    }
    else if (kpNearOut)
    {
        m_connectors.emplace(AttachableConnectorType::KP_DROPPER_OUT, *kpNearOut);
    }
}

DropperClamp::DropperClamp(std::string nodeName, double distance)
    : MorticeAttachable(nodeName, distance, /*useDroppers=*/false, /*isFlipped=*/false)
{
    fillConnectorsInfo();
}

void
DropperClamp::fillConnectorsInfo()
{
    const std::string path = nodeName();

    size_t const flippedIdx = isFlipped() ? 1 : 0;
    auto connectorsIter = s_insulatorConnectorsByPath.find(path);
    if (connectorsIter == s_insulatorConnectorsByPath.end())
    {
        auto iterPair = s_insulatorConnectorsByPath.emplace(path,
            FlippingConnectorPositionByType{
                fillConnectorsInfoForDropperClamp(path, /*flipped=*/false),
                fillConnectorsInfoForDropperClamp(path, /*flipped=*/true)});
        m_connectors = iterPair.first->second[flippedIdx];
    }
    else
    {
        m_connectors = connectorsIter->second[flippedIdx];
    }
}
