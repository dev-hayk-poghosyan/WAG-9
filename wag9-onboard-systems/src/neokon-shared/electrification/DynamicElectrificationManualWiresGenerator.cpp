#include "DynamicElectrificationManualWiresGenerator.h"

#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"
#include "neokon-shared/electrification/DynamicElectrification.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryGenerator.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/TransformTurnaroundZ.h"
#include <UnigineMathLib.h>
#include <UnigineWorld.h>

using namespace Unigine;
using namespace electrification;

bool operator==(DynamicElectrificationManualWiresGenerator::CreateArgs const& left,
                DynamicElectrificationManualWiresGenerator::CreateArgs const& right)
{
    if (left.wiresGenInfo != right.wiresGenInfo)
    {
        return false;
    }

    return true;
}

bool operator!=(DynamicElectrificationManualWiresGenerator::CreateArgs const& left,
                DynamicElectrificationManualWiresGenerator::CreateArgs const& right)
{
    return !(left == right);
}

std::optional<PoleCantileverConnectors>
findCantileverWithIDs(PoleSourceNodeHelperData const& helperData,
    std::list<std::string> const& idList)
{
    for (std::string const& id : idList)
    {
        auto findIt = helperData.connectorByCantilevers.find(id);
        if (findIt != helperData.connectorByCantilevers.cend())
        {
            return findIt->second;
        }
    }

    return std::nullopt;
}

std::optional<std::string>
getPoleNodeNameByGenInfo(DynamicPoleGenInfo const& poleGenInfo)
{
    auto generatedHost = DynamicElectrification::instance();
    assert(generatedHost);

    auto poleChooser =
        generatedHost->getOrCreateNodeNameChooser(poleGenInfo.preset);
    if (!poleChooser)
    {
        return std::nullopt;
    }

    std::optional<std::string> poleNodeName;

    if (poleGenInfo.poleType == PoleType::PortalPillar)
    {
        assert(poleGenInfo.portalPillarSide);

        PortalPillarSide side = *poleGenInfo.portalPillarSide;
        if (side == PortalPillarSide::Left)
        {
            poleNodeName = poleChooser->chooseLeftPortalPoleNodeName(PortalContextData{});
        }
        else
        {
            poleNodeName = poleChooser->chooseRightPortalPoleNodeName(PortalContextData{});
        }
    }
    else if (poleGenInfo.isSuspension)
    {
        poleNodeName = poleChooser->choosePortalSuspensionNodeName(poleGenInfo);
    }
    else
    {
        poleNodeName = poleChooser->choosePoleNodeName(poleGenInfo);
    }

    return poleNodeName;
}

std::optional<PoleSourceNodeHelperData>
getPoleNodeSourceHelperDataByGenInfo(DynamicPoleGenInfo const& poleGenInfo)
{
    if (std::optional<std::string> poleNodeName = getPoleNodeNameByGenInfo(poleGenInfo))
    {
        fillPoleSourceNodeHelperData(*poleNodeName);
        return findPoleNodeHelperData(*poleNodeName);
    }

    return std::nullopt;
}

std::optional<Math::Mat4>
getTensionTargetTransform(DynamicManualWireConnectInfo const& info)
{
    if (auto targetPoleHelperData = getPoleNodeSourceHelperDataByGenInfo(info.poleGenInfo))
    {
        if (auto cantilever = findCantileverWithIDs(*targetPoleHelperData,
                std::list<std::string>{info.cantileverGroup}))
        {
            return info.poleGenInfo.transform * cantilever->cable;
        }
    }

    return std::nullopt;
}

std::optional<Math::Mat4> getTensionLocalMountTransform(
    DynamicPoleGenInfo const& sourcePoleInfo)
{
    if (auto poleSourceData = getPoleNodeSourceHelperDataByGenInfo(sourcePoleInfo))
    {
        return poleSourceData->tensioningMountPoint;
    }

    return std::nullopt;
}

std::optional<Math::Mat4>
getTensionWorldMountTransform(DynamicPoleGenInfo const& sourcePoleInfo)
{
    if (auto localMountTransform = getTensionLocalMountTransform(sourcePoleInfo))
    {
        return sourcePoleInfo.transform * (*localMountTransform);
    }

    return std::nullopt;
}

Math::Mat4 getRotatedMountTransform(Math::Mat4 const& mountTransform,
    Math::Mat4 const& targetTransform)
{
    Math::Vec3 sourcePoint = mountTransform.getTranslate();
    Math::Vec3 dstPoint = targetTransform.getTranslate();
    Math::Vec3 directionFrom = mountTransform.getAxisX();
    Math::Vec3 directionTo = dstPoint - sourcePoint;
    directionFrom.z = directionTo.z = 0.0;
    directionFrom.normalize();
    directionTo.normalize();

    float angleDegrees = Math::getAngle(Math::vec3(directionFrom), Math::vec3(directionTo));
    Math::Vec3 rotationAxis = Math::cross(directionFrom, directionTo);
    Math::quat rotationQuat{Math::vec3{rotationAxis}, Math::abs(angleDegrees)};
    Math::Mat4 rotationTransform{rotationQuat};

    Math::Mat4 result = mountTransform * rotationTransform;

    return result;
}

void addAnchorBlocksToPoleObject(
    DynamicPoleGenInfo const& poleGenInfo,
    TensioningType tensioningType,
    Math::Mat4 const& tensioningMountTransform,
    std::function<void(std::string const&, Math::Mat4 const&)> const& sink)
{
    DynamicElectrification* generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(poleGenInfo.preset);
    if (!nodeNameChooser)
    {
        return;
    }

    AnchorContextData args{tensioningType, false};
    std::optional<std::string> anchorBlockNodeName =
        nodeNameChooser->chooseAnchorBlockNodeName(args);
    std::optional<std::string> backTieNodeName = nodeNameChooser->chooseAnchorBackTieNodeName(args);

    if (backTieNodeName)
    {
        sink(*backTieNodeName, tensioningMountTransform);
    }

    if (anchorBlockNodeName)
    {
        sink(*anchorBlockNodeName, tensioningMountTransform);
    }
}

DynamicElectrificationManualWiresGenerator::DynamicElectrificationManualWiresGenerator(uint32_t id,
    CreateArgs&& args)
    : DynamicElectrificationPart(id, 0)
    , m_genParams(std::move(args))
{
    setBBox(fromGlm(m_genParams.aabb.minCoords()), fromGlm(m_genParams.aabb.maxCoords()));

    auto fillNodeHelperData = [](DynamicManualWireConnectInfo& wireConnectInfo)
    {
        if (std::optional<std::string> poleNodeName =
                getPoleNodeNameByGenInfo(wireConnectInfo.poleGenInfo))
        {
            fillPoleSourceNodeHelperData(*poleNodeName);
        }
    };

    for (auto wireGenInfo : m_genParams.wiresGenInfo)
    {
        fillNodeHelperData(wireGenInfo.left);
        fillNodeHelperData(wireGenInfo.right);
    }
}

bool
DynamicElectrificationManualWiresGenerator::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    if (!old)
    {
        return true;
    }

    auto oldManualWiresGenerator = std::dynamic_pointer_cast<DynamicElectrificationManualWiresGenerator>(old);
    if (!oldManualWiresGenerator)
    {
        return true;
    }

    return getUniqueGenValue() != oldManualWiresGenerator->getUniqueGenValue();
}

bool
DynamicElectrificationManualWiresGenerator::load()
{
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLogger log("DynamicElectrificationManualWiresGenerator::load");
    ObjectMessageLoggerGuard log_guard(log);
#endif

    auto generatedHost = DynamicElectrification::instance();
    assert(generatedHost);

    auto getPoleWireConnectors = [generatedHost](
        DynamicManualWireConnectInfo const& wireConnectInfo,
        DynamicManualWireConnectInfo const& targetInfo)
        -> std::optional<PoleCantileverConnectors>
    {
        auto poleHelperData = getPoleNodeSourceHelperDataByGenInfo(wireConnectInfo.poleGenInfo);
        if (!poleHelperData)
        {
            return std::nullopt;
        }

        if (wireConnectInfo.tensioningType == TensioningType::None)
        {
            auto findIt =
                poleHelperData->connectorByCantilevers.find(wireConnectInfo.cantileverGroup);
            if (findIt == poleHelperData->connectorByCantilevers.cend())
            {
                return std::nullopt;
            }

            return findIt->second;
        }

        auto nodeNameChooser = generatedHost->getOrCreateNodeNameChooser(
            wireConnectInfo.poleGenInfo.preset);
        if (!nodeNameChooser)
        {
            return std::nullopt;
        }

        auto anchorBlockNodeName = nodeNameChooser->chooseAnchorBlockNodeName(
            AnchorContextData{wireConnectInfo.tensioningType, false });
        if (!anchorBlockNodeName)
        {
            return std::nullopt;
        }

        fillPoleSourceNodeHelperData(*anchorBlockNodeName);
        auto anchorBlockHelperData = findPoleNodeHelperData(*anchorBlockNodeName);
        if (!anchorBlockHelperData || !poleHelperData->tensioningMountPoint)
        {
            return std::nullopt;
        }

        auto mountLocalCantilever = findCantileverWithIDs(*anchorBlockHelperData,
            std::list<std::string>{POLE_CONNECTOR_ID_NT_WC});
        if (!mountLocalCantilever)
        {
            return std::nullopt;
        }

        auto worldMountTransform = getTensionWorldMountTransform(wireConnectInfo.poleGenInfo);
        auto targetWorldTransform = getTensionTargetTransform(targetInfo);
        if (!worldMountTransform || !targetWorldTransform)
        {
            return std::nullopt;
        }

        Math::Mat4 rotatedMountTransform = getRotatedMountTransform(
            *worldMountTransform, *targetWorldTransform);
        rotatedMountTransform = transformTurnaroundZ(rotatedMountTransform);
        Math::Mat4 invPoleTransform = Math::inverse(wireConnectInfo.poleGenInfo.transform);

        PoleCantileverConnectors result;
        result.cable = rotatedMountTransform * mountLocalCantilever->cable;
        result.cable = invPoleTransform * result.cable;

        if (mountLocalCantilever->wireFar)
        {
            result.wireFar = rotatedMountTransform * (*mountLocalCantilever->wireFar);
            result.wireFar = invPoleTransform * (*result.wireFar);
        }

        if (mountLocalCantilever->wireNear)
        {
            result.wireNear = rotatedMountTransform * (*mountLocalCantilever->wireNear);
            result.wireNear = invPoleTransform * (*result.wireNear);
        }

        return result;
    };

    for (auto wireGenInfo : m_genParams.wiresGenInfo)
    {
        DynamicPoleGenInfo const& leftPoleGenInfo = wireGenInfo.left.poleGenInfo;
        DynamicPoleGenInfo const& rightPoleGenInfo = wireGenInfo.right.poleGenInfo;

        auto leftPoleConnectors = getPoleWireConnectors(wireGenInfo.left, wireGenInfo.right);
        auto rightPoleConnectors = getPoleWireConnectors(wireGenInfo.right, wireGenInfo.left);

        if (!leftPoleConnectors || !rightPoleConnectors)
        {
            continue;
        }

        bool leftStitch = wireGenInfo.left.catenaryType == CatenaryType::Stitched;
        bool rightStitch = wireGenInfo.right.catenaryType == CatenaryType::Stitched;

        DynamicElectrificationCatenaryGenerator::CreateArgs catenaryGenArgs;
        catenaryGenArgs.defaultCantileverPreset = leftPoleGenInfo.preset;
        catenaryGenArgs.creationMode = m_genParams.creationMode;
        catenaryGenArgs.contextData =
            DynamicElectrificationCatenaryGenerator::connectTwoCantileversByCatenary(
                DynamicElectrificationCatenaryGenerator::ConnectTwoCantileversArgs{
                    *leftPoleConnectors, *rightPoleConnectors,
                    wireGenInfo.left.transform, wireGenInfo.right.transform,
                    leftStitch, rightStitch
                });

        auto poleChooser = generatedHost->getOrCreateNodeNameChooser(
            leftPoleGenInfo.preset);
        if (poleChooser)
        {
            if (auto name = poleChooser->chooseCatenaryAttachableNodeName(
                    CatenaryAttachableType::Dropper, CatenaryAttachableLength::Short))
            {
                catenaryGenArgs.contextData.dropperNodeName[CatenaryAttachableLength::Short] =
                    *name;
            }

            if (auto name = poleChooser->chooseCatenaryAttachableNodeName(
                    CatenaryAttachableType::Dropper, CatenaryAttachableLength::Medium))
            {
                catenaryGenArgs.contextData.dropperNodeName[CatenaryAttachableLength::Medium] =
                    *name;
            }

            if (auto name = poleChooser->chooseCatenaryAttachableNodeName(
                    CatenaryAttachableType::Dropper, CatenaryAttachableLength::Long))
            {
                catenaryGenArgs.contextData.dropperNodeName[CatenaryAttachableLength::Long] =
                    *name;
            }
        }

        auto catenaryGenerator =
            std::make_shared<DynamicElectrificationCatenaryGenerator>(
                m_key.id, std::move(catenaryGenArgs));
        if (catenaryGenerator)
        {
            catenaryGenerator->load();
            m_catenarySegments.push_back(catenaryGenerator);
        }
    }

    return true;
}

void
DynamicElectrificationManualWiresGenerator::unload()
{
    DynamicElectrificationPart::unload();

    if (m_rootNode)
    {
        m_rootNode.deleteForce();
    }
}

Unigine::NodePtr
DynamicElectrificationManualWiresGenerator::createNodes()
{
    auto addChildNode = [this]
    (NodePtr const& childNode, Math::Mat4 const& transform)
    {
        m_rootNode->addChild(childNode);
        childNode->setWorldTransform(transform);
    };

    auto addAnchorNodeSink = [this, &addChildNode]
    (std::string const& nodePath, Math::Mat4 const& transform)
    {
        NodePtr anchorNode = World::loadNode(nodePath.c_str());
        if (!anchorNode)
        {
            error("Overlap generator: couldn't load node \"%s\"", nodePath.c_str());
            return;
        }

        addChildNode(anchorNode, transform);
    };

    maybeCreateRootNode();

    Math::Mat4 rootITransform = Math::inverse(m_rootNode->getWorldTransform());
    for (auto const& catenarySegment : m_catenarySegments)
    {
        NodePtr catenaryNode = catenarySegment->createNodes();
        addChildNode(catenaryNode, catenarySegment->globalTransform());
    }

    if (m_genParams.creationMode == CreationMode::NODES)
    {
        for (auto wireGenInfo : m_genParams.wiresGenInfo)
        {
            if (wireGenInfo.left.tensioningType != TensioningType::None)
            {
                auto worldMountTransform = getTensionWorldMountTransform(wireGenInfo.left.poleGenInfo);
                auto targetTransform = getTensionTargetTransform(wireGenInfo.right);
                if (!worldMountTransform || !targetTransform)
                {
                    continue;
                }
                Math::Mat4 mountTransform = getRotatedMountTransform(*worldMountTransform, *targetTransform);
                addAnchorBlocksToPoleObject(wireGenInfo.left.poleGenInfo,
                                            wireGenInfo.left.tensioningType, mountTransform, addAnchorNodeSink);
            }

            if (wireGenInfo.right.tensioningType != TensioningType::None)
            {
                auto worldMountTransform = getTensionWorldMountTransform(wireGenInfo.right.poleGenInfo);
                auto targetTransform = getTensionTargetTransform(wireGenInfo.left);
                if (!worldMountTransform || !targetTransform)
                {
                    continue;
                }
                Math::Mat4 mountTransform =
                    getRotatedMountTransform(*worldMountTransform, *targetTransform);
                addAnchorBlocksToPoleObject(wireGenInfo.right.poleGenInfo,
                                            wireGenInfo.right.tensioningType, mountTransform, addAnchorNodeSink);
            }
        }
    }

    onGenerationFinished(m_rootNode);
    return m_rootNode;
}

void
DynamicElectrificationManualWiresGenerator::validateClusters(
    GetOrCreateClusterFunc const& getOrCreateCluster,
    std::optional<Unigine::Math::Vec3> const& center) const
{
    auto addClusterNodeInstance =
        [&getOrCreateCluster, &center](std::string const& nodeName, Math::Mat4 const& transform)
    {
        if (auto cluster = getOrCreateCluster(nodeName,
                /*matInfo=*/RailwayMeshGeneratorConfig::MaterialInfo{}))
        {
            if (center)
            {
                double distance = (transform.getTranslate() - (*center)).length();
                double visibleDistance = cluster->getVisibleDistance() * 2.0f;
                if (distance > visibleDistance)
                {
                    return;
                }
            }

            const Math::Mat4 invClusterTransform = Math::inverse(cluster->getWorldTransform());
            const Math::Mat4 localClusterTransform = invClusterTransform * transform;
            const int instanceIdx = cluster->addMeshTransform();
            cluster->setMeshTransform(instanceIdx, Math::mat4{localClusterTransform});
        }
    };

    for (auto const& catenarySegment : m_catenarySegments)
    {
        catenarySegment->validateClusters(getOrCreateCluster, center);
    }

    if (m_genParams.creationMode == CreationMode::CLUSTER)
    {
        for (auto wireGenInfo : m_genParams.wiresGenInfo)
        {
            if (wireGenInfo.left.tensioningType != TensioningType::None)
            {
                auto worldMountTransform = getTensionWorldMountTransform(wireGenInfo.left.poleGenInfo);
                auto targetTransform = getTensionTargetTransform(wireGenInfo.right);
                if (!worldMountTransform || !targetTransform)
                {
                    continue;
                }
                Math::Mat4 mountTransform =
                    getRotatedMountTransform(*worldMountTransform, *targetTransform);
                addAnchorBlocksToPoleObject(wireGenInfo.left.poleGenInfo,
                                            wireGenInfo.left.tensioningType, mountTransform, addClusterNodeInstance);
            }

            if (wireGenInfo.right.tensioningType != TensioningType::None)
            {
                auto worldMountTransform = getTensionWorldMountTransform(wireGenInfo.right.poleGenInfo);
                auto targetTransform = getTensionTargetTransform(wireGenInfo.left);
                if (!worldMountTransform || !targetTransform)
                {
                    continue;
                }
                Math::Mat4 mountTransform =
                    getRotatedMountTransform(*worldMountTransform, *targetTransform);
                addAnchorBlocksToPoleObject(wireGenInfo.right.poleGenInfo,
                                            wireGenInfo.right.tensioningType, mountTransform, addClusterNodeInstance);
            }
        }//for (auto wireGenInfo : m_genParams.wiresGenInfo)
    }//if (m_genParams.creationMode == CreationMode::CLUSTER)
}

void
DynamicElectrificationManualWiresGenerator::maybeCreateRootNode()
{
    if (!m_rootNode)
    {
        String nodeName = String::format("manual_wire_%d", m_key.id);
        Math::Vec3 rootPosition = getBoundBox().getCenter();

        m_rootNode = NodeDummy::create();
        m_rootNode->setName(nodeName);
        m_rootNode->setWorldTransform(Math::translate(rootPosition));
    }
}
