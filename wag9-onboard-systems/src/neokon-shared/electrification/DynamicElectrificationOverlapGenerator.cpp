#include "DynamicElectrificationOverlapGenerator.h"

#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/utils/ElectrificationNodeConnectorsData.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"
#include "neokon-shared/electrification/DynamicElectrification.h"
#include "neokon-shared/electrification/DynamicElectrificationPoleGenerator.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/TransformTurnaroundZ.h"
#include "neokon-shared/TraverseNodeHierarchy.h"

#include <UnigineMathLib.h>
#include <UnigineWorld.h>

#include <chrono>

using namespace Unigine;
using namespace electrification;

bool
operator==(DynamicElectrificationOverlapGenerator::CreateArgs const& left,
    DynamicElectrificationOverlapGenerator::CreateArgs const& right)
{
    if (left.defaultCantileverPreset != right.defaultCantileverPreset)
    {
        return false;
    }

    if (left.startTensioningType != right.startTensioningType)
    {
        return false;
    }

    if (left.endTensioningType != right.endTensioningType)
    {
        return false;
    }

    if (left.catenaryType != right.catenaryType)
    {
        return false;
    }

    if (left.poleGenInfo != right.poleGenInfo)
    {
        return false;
    }

    return true;
}

bool
operator!=(DynamicElectrificationOverlapGenerator::CreateArgs const& left,
    DynamicElectrificationOverlapGenerator::CreateArgs const& right)
{
    return !(left == right);
}

namespace
{
std::optional<PoleSourceNodeHelperData> getAnchorBlocksNodeHelperData(DynamicPoleGenInfo const& genInfo,
    std::optional<TensioningType> tensioningType = std::nullopt)
{
    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(genInfo.preset);
    if (!nodeNameChooser)
    {
        return std::nullopt;
    }

    std::optional<std::string> nodePath;
    if (tensioningType)
    {
        AnchorContextData anchorData{*tensioningType};
        nodePath = nodeNameChooser->chooseAnchorBlockNodeName(anchorData);
    }
    else
    {
        nodePath = genInfo.isSuspension
            ? nodeNameChooser->choosePortalSuspensionNodeName(genInfo)
            : nodeNameChooser->choosePoleNodeName(genInfo);
    }

    if (nodePath)
    {
        fillPoleSourceNodeHelperData(*nodePath);
        return findPoleNodeHelperData(*nodePath);
    }

    return std::nullopt;
}

std::optional<PoleCantileverConnectors> findCantileverWithIDs(
    PoleSourceNodeHelperData const& helperData, std::list<std::string> const& idList)
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

} // anonymous namespace

DynamicElectrificationOverlapGenerator::DynamicElectrificationOverlapGenerator(uint32_t id,
    CreateArgs&& args)
    : DynamicElectrificationPart(id, 0)
    , m_genParams(std::move(args))
{
    setBBox(fromGlm(m_genParams.aabb.minCoords()), fromGlm(m_genParams.aabb.maxCoords()));

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    for (auto const& info : m_genParams.poleGenInfo)
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
        assert(nodeNameChooser);

        auto config = generatorsHost->getOrCreateMeshConfig(info.preset);
        assert(config);

        std::optional<std::string> poleNodeName = info.isSuspension
            ? nodeNameChooser->choosePortalSuspensionNodeName(info)
            : nodeNameChooser->choosePoleNodeName(info);
        if (!poleNodeName)
        {
            continue;
        }

        fillPoleSourceNodeHelperData(*poleNodeName);

        auto const& catenaryData = config->getCatenaryData();
        fillNodeConnectorsData(catenaryData.sectionInsulator.nodeLower);
        fillNodeConnectorsData(catenaryData.sectionInsulator.nodeUpper);
        fillNodeConnectorsData(catenaryData.tensionInsulator.nodeLower);
        fillNodeConnectorsData(catenaryData.tensionInsulator.nodeUpper);
        fillNodeConnectorsData(catenaryData.overlapInsulator.nodeLower);
        fillNodeConnectorsData(catenaryData.overlapInsulator.nodeUpper);
    }

    loadTensioningMounts();
    loadAnchorBlocksCatenaryData();
}

bool
DynamicElectrificationOverlapGenerator::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    if (!old)
    {
        return true;
    }

    auto oldPoleGenerator = std::dynamic_pointer_cast<DynamicElectrificationOverlapGenerator>(old);
    if (!oldPoleGenerator)
    {
        return true;
    }

    auto const& otherParams = oldPoleGenerator->m_genParams;

    return otherParams != m_genParams;
}

bool
DynamicElectrificationOverlapGenerator::load()
{
    assert(m_genParams.poleGenInfo.size() >= 4);

    //loadTensioningMounts();
    // Requires calculations in the main thread (loading the Unigine Node).
    // So we cannot call it here.
    //loadAnchorBlocksCatenaryData();

    if (m_genParams.generateWires)
    {
        loadCatenarySegments();
    }

    return true;
}

void
DynamicElectrificationOverlapGenerator::unload()
{
    DynamicElectrificationPart::unload();

    if (m_rootNode)
    {
        m_rootNode.deleteForce();
    }
}

Unigine::NodePtr
DynamicElectrificationOverlapGenerator::createNodes()
{
    if (m_genParams.poleGenInfo.size() == 0)
    {
        return nullptr;
    }

    maybeCreateRootNode();

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    for (auto& catenarySegment : m_catenarySegments)
    {
        if (auto wireNode = catenarySegment->rootNode())
        {
            wireNode.deleteLater();
        }
    }

    if (m_genParams.generateWires)
    {
        for (auto const& catenarySegment : m_catenarySegments)
        {
            NodePtr catenaryNode = catenarySegment->createNodes();

            m_rootNode->addChild(catenaryNode);
            catenaryNode->setWorldTransform(catenarySegment->globalTransform());
        }
    }

    auto addChildNode = [this](std::string const& nodePath, Math::Mat4 const& transform)
    {
        if (nodePath.empty())
        {
            // TODO: maybe log error.
            return;
        }
        NodePtr childNode = World::loadNode(nodePath.c_str());
        if (!childNode)
        {
            error("Overlap generator: couldn't load node \"%s\"", nodePath.c_str());
            return;
        }

        if (m_genParams.creationMode == CreationMode::NODES)
        {
            m_rootNode->addChild(childNode);
            childNode->setWorldTransform(transform);
        }
        else
        {
            traverseNodeHierarchy(childNode,
                [this, &transform](Unigine::NodePtr node)
                {
                    if (node->getType() >= Node::TYPE::DECAL_BEGIN &&
                        node->getType() <= Node::TYPE::DECAL_END)
                    {
                        Math::Mat4 const initialTransform(node->getWorldTransform());
                        m_rootNode->addChild(node);
                        Math::Mat4 const finalWorldTransform(transform * initialTransform);
                        node->setWorldTransform(finalWorldTransform);
                    }
                });
            childNode.deleteLater();
        }
    };

    int firstPoleIdx = 1;
    int lastPoleIdx = m_genParams.poleGenInfo.size() - 2;
    assert(firstPoleIdx < m_genParams.poleGenInfo.size());
    assert(lastPoleIdx >= firstPoleIdx);
    for (int i = firstPoleIdx; i <= lastPoleIdx; i++)
    {
        createPole(m_genParams.poleGenInfo[i], addChildNode);
    }

    createAnchors(m_genParams.poleGenInfo, addChildNode);

    onGenerationFinished(m_rootNode);
    return m_rootNode;
}

void
DynamicElectrificationOverlapGenerator::validateClusters(
    GetOrCreateClusterFunc const& getOrCreateCluster,
    std::optional<Unigine::Math::Vec3> const& center) const
{
    if (m_genParams.creationMode != CreationMode::CLUSTER)
    {
        return;
    }

    if (m_genParams.poleGenInfo.size() == 0)
    {
        return;
    }

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

    auto addClusterNodeInstanceFlipped =
        [&getOrCreateCluster, &center](std::string const& nodeName, Math::Mat4 const& transform)
    {
        if (auto cluster = getOrCreateCluster(nodeName,
                /*matInfo=*/RailwayMeshGeneratorConfig::MaterialInfo{}))
        {
            Math::Mat4 const rotatedTransform(transformTurnaroundZ(transform));

            if (center)
            {
                double distance = (rotatedTransform.getTranslate() - (*center)).length();
                double visibleDistance = cluster->getVisibleDistance() * 2.0f;
                if (distance > visibleDistance)
                {
                    return;
                }
            }

            const Math::Mat4 invClusterTransform = Math::inverse(cluster->getWorldTransform());
            const Math::Mat4 localClusterTransform = invClusterTransform * rotatedTransform;
            const int instanceIdx = cluster->addMeshTransform();
            cluster->setMeshTransform(instanceIdx, Math::mat4{localClusterTransform});
        }
    };

    int firstPoleIdx = 1;
    int lastPoleIdx = m_genParams.poleGenInfo.size() - 2;
    assert(firstPoleIdx < m_genParams.poleGenInfo.size());
    assert(lastPoleIdx >= firstPoleIdx);
    for (int i = firstPoleIdx; i <= lastPoleIdx; i++)
    {
        createPole(m_genParams.poleGenInfo[i], addClusterNodeInstance);
    }

    if (m_genParams.generateWires)
    {
        for (auto const& catenarySegment : m_catenarySegments)
        {
            catenarySegment->validateClusters(getOrCreateCluster, center);
        }
    }

    createAnchors(m_genParams.poleGenInfo, addClusterNodeInstanceFlipped);
}

void
DynamicElectrificationOverlapGenerator::loadCatenarySegments()
{
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLogger log("DynamicElectrificationOverlapGenerator::loadCatenarySegments");
    ObjectMessageLoggerGuard log_guard(log);
#endif

    int numPoles = m_genParams.poleGenInfo.size();
    assert(numPoles >= 2);

    auto makeGenCatenaryArgs = [this](CatenaryContextData const& catenaryData)
        -> DynamicElectrificationCatenaryGenerator::CreateArgs
    {
        DynamicElectrificationCatenaryGenerator::CreateArgs result;
        result.creationMode = m_genParams.creationMode;
        result.defaultCantileverPreset = m_genParams.defaultCantileverPreset;
        result.contextData = catenaryData;

        return result;
    };

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLogger* pLog = &log;
#else
    ObjectMessageLogger* pLog = nullptr;
#endif

    std::list<DynamicElectrificationCatenaryGenerator::CreateArgs> genDataList;
    for (int i = 1; i < numPoles; i++)
    {
        // The last wire group in the overlap is inverted.
        auto const& rightPole =
            (i == numPoles - 1) ? m_genParams.poleGenInfo[i - 1] : m_genParams.poleGenInfo[i];
        auto const& leftPole =
            (i == numPoles - 1) ? m_genParams.poleGenInfo[i] : m_genParams.poleGenInfo[i - 1];

        DynamicElectrificationCatenaryGenerator::ConnectTwoPolesByCatenaryArgs connectArgs{
            leftPole, rightPole, pLog};
        connectArgs.leftPolePreset = leftPole.preset;
        connectArgs.rightPolePreset = rightPole.preset;
        connectArgs.creationMode = m_genParams.creationMode;
        connectArgs.defaultCantileverPreset = m_genParams.defaultCantileverPreset;

        if (i == 1)
        {
            connectArgs.leftStitch = m_genParams.startCatenaryType == CatenaryType::Stitched;
            connectArgs.rightStitch = m_genParams.catenaryType == CatenaryType::Stitched;
        }
        else if (i == numPoles - 1)
        {
            connectArgs.leftStitch = m_genParams.endCatenaryType == CatenaryType::Stitched;
            connectArgs.rightStitch = m_genParams.catenaryType == CatenaryType::Stitched;
        }
        else
        {
            connectArgs.leftStitch = m_genParams.catenaryType == CatenaryType::Stitched;
            connectArgs.rightStitch = m_genParams.catenaryType == CatenaryType::Stitched;
        }

        auto resultList
            = DynamicElectrificationCatenaryGenerator::connectTwoPolesByCatenary(connectArgs);
        if (resultList.size() == 0)
        {
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
            log.error("Can not generate catenary between %d and %d poles within tension object #%d\n",
                (i - 1), i, getKey().id);
#endif
            continue;
        }

        genDataList.insert(genDataList.cend(), resultList.cbegin(), resultList.cend());
    }

    if (m_startAnchorCatenaryData)
    {
        DynamicElectrificationCatenaryGenerator::CreateArgs connectArgs =
            makeGenCatenaryArgs(*m_startAnchorCatenaryData);
        genDataList.push_back(connectArgs);
    }

    if (m_endAnchorCatenaryData)
    {
        DynamicElectrificationCatenaryGenerator::CreateArgs connectArgs =
            makeGenCatenaryArgs(*m_endAnchorCatenaryData);
        int lastIdx = m_genParams.poleGenInfo.size() - 1;
        genDataList.push_back(connectArgs);
    }

    for (auto& catenarySegment : m_catenarySegments)
    {
        if (auto wireNode = catenarySegment->rootNode())
        {
            wireNode.deleteLater();
        }
    }
    m_catenarySegments.clear();

    for (auto&& createArgs : genDataList)
    {
        auto catenaryGenerator = std::make_shared<DynamicElectrificationCatenaryGenerator>(m_key.id,
            std::move(createArgs));
        if (catenaryGenerator)
        {
            catenaryGenerator->load();
            m_catenarySegments.emplace_back(catenaryGenerator);
        }
    } // for (auto&& createArgs : genDataList)
}

void DynamicElectrificationOverlapGenerator::loadTensioningMounts()
{
    auto genHost = DynamicElectrification::instance();
    assert(genHost);

    auto getHelperData = [&genHost](DynamicPoleGenInfo const& genInfo)
        -> std::optional<PoleSourceNodeHelperData>
    {
        auto chooser = genHost->getOrCreateNodeNameChooser(genInfo.preset);
        if (!chooser)
        {
            return std::nullopt;
        }

        auto poleNodeName = genInfo.isSuspension
            ? chooser->choosePortalSuspensionNodeName(genInfo)
            : chooser->choosePoleNodeName(genInfo);
        if (!poleNodeName)
        {
            return std::nullopt;
        }

        return findPoleNodeHelperData(*poleNodeName);
    };

    auto getTensionMountTransform =
        [&getHelperData](DynamicPoleGenInfo const& genInfo)
        -> std::optional<Math::Mat4>
    {
        if (genInfo.isSuspension)
        {
            //TODO: временное решение. Если крайняя опора сопряжения - подвес под поперечиной,
            // то анкеровку не отображаем.
            return std::nullopt;
        }

        if (auto helperData = getHelperData(genInfo))
        {
            if (helperData->tensioningMountPoint)
            {
                return genInfo.transform * (*helperData->tensioningMountPoint);
            }
        }

        return std::nullopt;
    };

    auto getWAConnectorTransform =
        [&getHelperData](electrification::DynamicPoleGenInfo const& genInfo)
        -> std::optional<Math::Mat4>
    {
        auto helperData = getHelperData(genInfo);
        if (!helperData)
        {
            return std::nullopt;
        }

        const std::list<std::string> waConnectors
        {
            POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WN
        };

        for (std::string const& key : waConnectors)
        {
            if (helperData->connectorByCantilevers.count(key) == 0)
            {
                continue;
            }

            auto const& cantilever = helperData->connectorByCantilevers.at(key);

            return genInfo.transform * cantilever.cable;
        };

        return std::nullopt;
    };

    auto getRotatedMountTransform =
        [] (Math::Mat4 const& mountTransform, Math::Mat4 const& waConnectorTransform)
    {
        Math::Vec3 sourcePoint = mountTransform.getTranslate();
        Math::Vec3 dstPoint = waConnectorTransform.getTranslate();
        Math::Vec3 directionFrom = mountTransform.getAxisX();
        Math::Vec3 directionTo = sourcePoint - dstPoint;
        directionFrom.z = directionTo.z = 0.0;
        directionFrom.normalize();
        directionTo.normalize();

        float angleDegrees = Math::getAngle(Math::vec3(directionFrom), Math::vec3(directionTo));
        Math::Vec3 rotationAxis = Math::cross(directionFrom, directionTo);
        Math::quat rotationQuat{ Math::vec3{ rotationAxis }, Math::abs(angleDegrees) };
        Math::Mat4 rotationTransform{ rotationQuat };

        return mountTransform * rotationTransform;
    };

    assert(m_genParams.poleGenInfo.size() >= 4);
    int poleAIdx = 1;
    int poleBIdx = m_genParams.poleGenInfo.size() - 2;
    auto startMountTransform = getTensionMountTransform(m_genParams.poleGenInfo.front());
    auto endMountTransform = getTensionMountTransform(m_genParams.poleGenInfo.back());
    auto poleWAConnectorA = getWAConnectorTransform(m_genParams.poleGenInfo[poleAIdx]);
    auto poleWAConnectorB = getWAConnectorTransform(m_genParams.poleGenInfo[poleBIdx]);

    if (startMountTransform && poleWAConnectorA)
    {
        m_startTensioningMount = getRotatedMountTransform(
            *startMountTransform, *poleWAConnectorA);
    }

    if (endMountTransform && poleWAConnectorB)
    {
        m_endTensioningMount = getRotatedMountTransform(
            *endMountTransform, *poleWAConnectorB);
    }
}

void DynamicElectrificationOverlapGenerator::loadAnchorBlocksCatenaryData()
{
    assert(m_genParams.poleGenInfo.size() >= 4);

    if (m_startTensioningMount)
    {
        m_startAnchorCatenaryData = getAnchorCatenaryData(
            m_genParams.poleGenInfo[0], m_genParams.poleGenInfo[1],
            m_genParams.startTensioningType, *m_startTensioningMount);
    }

    if (m_endTensioningMount)
    {
        int lastIdx = m_genParams.poleGenInfo.size() - 1;
        m_endAnchorCatenaryData = getAnchorCatenaryData(
            m_genParams.poleGenInfo[lastIdx], m_genParams.poleGenInfo[lastIdx - 1],
            m_genParams.endTensioningType, *m_endTensioningMount);
    }
}

std::optional<electrification::CatenaryContextData>
DynamicElectrificationOverlapGenerator::getAnchorCatenaryData(
    electrification::DynamicPoleGenInfo const& anchorPoleGenInfo,
    electrification::DynamicPoleGenInfo const& transitionalPoleGenInfo,
    electrification::TensioningType tensioningType, Math::Mat4 const tensioningMount)
{
    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto anchorBlockData = getAnchorBlocksNodeHelperData(anchorPoleGenInfo, tensioningType);
    assert(anchorBlockData);
    auto transitionalPoleData = getAnchorBlocksNodeHelperData(transitionalPoleGenInfo);
    assert(transitionalPoleData);

    auto anchorCantilever =
        findCantileverWithIDs(*anchorBlockData, std::list<std::string>{POLE_CONNECTOR_ID_NT_WC});
    assert(anchorCantilever);
    auto transitionalPoleCantilever = findCantileverWithIDs(*transitionalPoleData,
        std::list<std::string>{POLE_CONNECTOR_ID_NT_WA, POLE_CONNECTOR_ID_NT_WN});
    assert(transitionalPoleCantilever);

    electrification::CatenaryContextData contextData =
        DynamicElectrificationCatenaryGenerator::connectTwoCantileversByCatenary(
            DynamicElectrificationCatenaryGenerator::ConnectTwoCantileversArgs{
                *transitionalPoleCantilever, *anchorCantilever, transitionalPoleGenInfo.transform,
                tensioningMount});

    auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(anchorPoleGenInfo.preset);
    if (!nodeNameChooser)
    {
        error("Pole generator: cannot get name chooser for preset: %s\n",
            anchorPoleGenInfo.preset.c_str());
        return contextData;
    }

    auto writeNodeName = [&nodeNameChooser](electrification::CatenaryAttachableType attachableType,
                             electrification::CatenaryAttachableLength attachableLengthType,
                             CatenaryAttachableNodeName& nodeNameOutput)
    {
        if (auto name = nodeNameChooser->chooseCatenaryAttachableNodeName(attachableType,
                attachableLengthType))
        {
            nodeNameOutput[attachableLengthType] = *name;
        }
    };

    auto writeNodes =
        [&nodeNameChooser, &writeNodeName](electrification::CatenaryAttachableType attachableType,
            CatenaryAttachableNodeName& nodeNameOutput)
    {
        writeNodeName(attachableType, electrification::CatenaryAttachableLength::Short,
            nodeNameOutput);
        writeNodeName(attachableType, electrification::CatenaryAttachableLength::Medium,
            nodeNameOutput);
        writeNodeName(attachableType, electrification::CatenaryAttachableLength::Long,
            nodeNameOutput);
    };

    writeNodes(electrification::CatenaryAttachableType::Dropper, contextData.dropperNodeName);
    writeNodes(electrification::CatenaryAttachableType::Dropper1w, contextData.dropperNodeName1w);
    writeNodes(electrification::CatenaryAttachableType::InsulatorDropper,
        contextData.insulatorDropperNodeName);
    writeNodes(electrification::CatenaryAttachableType::CompensatedMidpointAnchor,
        contextData.compMidPointAnchorName);
    writeNodes(electrification::CatenaryAttachableType::SemiCompensatedMidpointAnchor,
        contextData.scompMidPointAnchorName);

    return contextData;
}

void DynamicElectrificationOverlapGenerator::maybeCreateRootNode()
{
    if (!m_rootNode)
    {
        String nodeName = String::format("tension_overlap_%d", m_key.id);
        Math::Vec3 rootPosition = getBoundBox().getCenter();

        m_rootNode = NodeDummy::create();
        m_rootNode->setName(nodeName);
        m_rootNode->setWorldPosition(rootPosition);
    }
}

void DynamicElectrificationOverlapGenerator::createPole(
    electrification::DynamicPoleGenInfo const& info, CreateInstanceFunc const& sink) const
{
    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
    assert(nodeNameChooser);

    auto poleNodeName = info.isSuspension ? nodeNameChooser->choosePortalSuspensionNodeName(info)
                                          : nodeNameChooser->choosePoleNodeName(info);
    if (!poleNodeName)
    {
        return;
    }

    sink(*poleNodeName, info.transform);
}

void
DynamicElectrificationOverlapGenerator::createAnchors(
    std::vector<electrification::DynamicPoleGenInfo> const& poleGenInfo,
    CreateInstanceFunc const& sink) const
{
    struct AddAnchorInfo
    {
        Math::Mat4 tensioningMount;
        int poleIdx{0};
        TensioningType tensioningType{TensioningType::None};

        AddAnchorInfo(Math::Mat4 const& mountTransform, int idx, TensioningType tType)
            : tensioningMount(mountTransform)
            , poleIdx(idx)
            , tensioningType(tType)
        {
        }
    };

    std::vector<AddAnchorInfo> endpointAnchors;

    if (m_startTensioningMount)
    {
        endpointAnchors.emplace_back(*m_startTensioningMount, 0, m_genParams.startTensioningType);
    }

    if (m_endTensioningMount)
    {
        endpointAnchors.emplace_back(*m_endTensioningMount, m_genParams.poleGenInfo.size() - 1,
            m_genParams.endTensioningType);
    }

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    for (auto const& endpointAnchor : endpointAnchors)
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(
            m_genParams.poleGenInfo[endpointAnchor.poleIdx].preset);
        assert(nodeNameChooser);

        AnchorContextData anchorContextData{endpointAnchor.tensioningType};

        if (auto anchorBlockNodeName =
                nodeNameChooser->chooseAnchorBlockNodeName(anchorContextData))
        {
            sink(*anchorBlockNodeName, endpointAnchor.tensioningMount);
        }

        if (auto backTieNodeName = nodeNameChooser->chooseAnchorBackTieNodeName(anchorContextData))
        {
            sink(*backTieNodeName, endpointAnchor.tensioningMount);
        }
    }
}
