#include "DynamicElectrificationPoleGenerator.h"

#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/DynamicElectrification.h"
#include "neokon-shared/electrification/utils/ElectrificationNodeConnectorsData.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"
#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/TransformTurnaroundZ.h"

#include <UnigineMathLib.h>
#include <UnigineWorld.h>

using namespace Unigine;
using namespace electrification;

bool operator==(DynamicElectrificationPoleGenerator::CreateArgs const& left,
                DynamicElectrificationPoleGenerator::CreateArgs const& right)
{
    if (left.defaultCantileverPreset != right.defaultCantileverPreset)
    {
        return false;
    }

    if (left.middleTensioning != right.middleTensioning)
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

    if (left.tensionPathPlacement != right.tensionPathPlacement)
    {
        return false;
    }

    return true;
}

bool operator!=(DynamicElectrificationPoleGenerator::CreateArgs const& left,
                DynamicElectrificationPoleGenerator::CreateArgs const& right)
{
    return !(left == right);
}

void renderWireConnectors(PoleContextData const& poleData,
    std::string const& polePreset)
{
    auto host = DynamicElectrification::instance();
    assert(host);
    auto nameChooser = host->getOrCreateNodeNameChooser(polePreset);
    assert(nameChooser);

    auto poleNodeName = nameChooser->choosePoleNodeName(poleData);
    if (!poleNodeName)
    {
        return;
    }

    auto helperData = findPoleNodeHelperData(*poleNodeName);
    if (!helperData)
    {
        return;
    }

    for (auto const& [_, connectorLocalBase] : helperData->connectorsByNames)
    {
        Math::Mat4 connectorWorldTransform = poleData.transform * connectorLocalBase;
        Math::Vec3 connectorWorldPosition = connectorWorldTransform.getTranslate();

#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
        host->getDebugVisualiser()->addPoint(connectorWorldPosition);
#endif
    }
}

DynamicElectrificationPoleGenerator::DynamicElectrificationPoleGenerator(uint32_t id,
    CreateArgs&& args)
    : DynamicElectrificationPart(id, 0)
    , m_genParams(std::move(args))
{
    if (m_genParams.generateWires && m_genParams.poleGenInfo.size() < 2)
    {
        // This conditions causes error because we might want to generate wires on the poles,
        // but the tension object has only a single pole.
        // TODO: process differently or remove.
        Unigine::Log::message("m_genParams.generateWires && m_genParams.poleGenInfo.size() < 2\n");
    }
    setBBox(fromGlm(m_genParams.aabb.minCoords()), fromGlm(m_genParams.aabb.maxCoords()));

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    for (auto const& info : m_genParams.poleGenInfo)
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
        if (!nodeNameChooser)
        {
            Unigine::Log::error("Couldn't get node name chooser for preset %s\n",
                info.preset.c_str());
            continue;
        }

        std::optional<std::string> poleNodeName = info.isSuspension
            ? nodeNameChooser->choosePortalSuspensionNodeName(info)
            : nodeNameChooser->choosePoleNodeName(info);
        if (!poleNodeName)
        {
            continue;
        }

        fillPoleSourceNodeHelperData(*poleNodeName);

        auto config = generatorsHost->getOrCreateMeshConfig(info.preset);
        assert(config);
        auto const& catenaryData = config->getCatenaryData();
        fillNodeConnectorsData(catenaryData.sectionInsulator.nodeLower);
        fillNodeConnectorsData(catenaryData.sectionInsulator.nodeUpper);
        fillNodeConnectorsData(catenaryData.tensionInsulator.nodeLower);
        fillNodeConnectorsData(catenaryData.tensionInsulator.nodeUpper);
        fillNodeConnectorsData(catenaryData.overlapInsulator.nodeLower);
        fillNodeConnectorsData(catenaryData.overlapInsulator.nodeUpper);
    }

    if (m_genParams.middleTensioning == TensioningType::Compensated)
    {
        loadTensioningMounts();
        loadAnchorBlocksCatenaryData();
    }
}

bool
DynamicElectrificationPoleGenerator::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    if (!old)
    {
        return true;
    }

    auto oldPoleGenerator = std::dynamic_pointer_cast<DynamicElectrificationPoleGenerator>(old);
    if (!oldPoleGenerator)
    {
        return true;
    }

    auto const& otherParams = oldPoleGenerator->m_genParams;

    return otherParams != m_genParams;
}

bool
DynamicElectrificationPoleGenerator::load()
{
    if (m_genParams.poleGenInfo.size() > 3)
    {
        size_t tail = m_genParams.poleGenInfo.size() % 2;
        m_middleAnchorPoleIdx = (m_genParams.poleGenInfo.size() - tail) / 2 - 1;
        m_startAnchorPoleIdx = m_middleAnchorPoleIdx - 1;
        m_endAnchorPoleIdx = m_middleAnchorPoleIdx + 1;
    }

    // Requires calculations in the main thread (loading the Unigine Node),
    // so now it's commented out.
    //if (m_genParams.middleTensioning == TensioningType::Compensated)
    //{
    //    loadTensioningMounts();
    //    loadAnchorBlocksCatenaryData();
    //}

    // Prevent errors when there we want to generate wires but have only a single pole.
    // There is a warning in the class constructor related to that.
    if (m_genParams.generateWires && m_genParams.poleGenInfo.size() > 1)
    {
        loadCatenarySegments();
    }

    return true;
}

void
DynamicElectrificationPoleGenerator::unload()
{
    DynamicElectrificationPart::unload();

    if (m_rootNode)
    {
        m_rootNode.deleteForce();
    }
}

Unigine::NodePtr
DynamicElectrificationPoleGenerator::createNodes()
{
    if (m_genParams.poleGenInfo.size() == 0)
    {
        warning("Pole generator: pole gen info is empty\n");
        return nullptr;
    }

    maybeCreateRootNode();

    auto addChildNode = [this](std::string const& poleNodeName, Math::Mat4 const& transform)
    {
        if (poleNodeName.empty())
        {
            // TODO: maybe log error.
            return;
        }
        NodePtr poleNode = World::loadNode(poleNodeName.c_str());
        if (!poleNode)
        {
            error("Pole generator: couldn't load node \"%s\"", poleNodeName.c_str());
            return;
        }
        m_rootNode->addChild(poleNode);
        poleNode->setWorldTransform(transform);
    };

    if (m_genParams.generateWires)
    {
        for (auto const& catenarySegment : m_catenarySegments)
        {
            NodePtr catenaryNode = catenarySegment->createNodes();
            m_rootNode->addChild(catenaryNode);
            catenaryNode->setWorldTransform(catenarySegment->globalTransform());
        }
    }

    if (m_genParams.creationMode == CreationMode::NODES)
    {
        createPoles(m_genParams.poleGenInfo, addChildNode);
        createAnchors(m_genParams.poleGenInfo, addChildNode);
    }

    onGenerationFinished(m_rootNode);
    return m_rootNode;
}

void
DynamicElectrificationPoleGenerator::validateClusters(
    GetOrCreateClusterFunc const& getOrCreateCluster,
    std::optional<Unigine::Math::Vec3> const& center) const
{
    if (m_genParams.creationMode != CreationMode::CLUSTER)
    {
        return;
    }

    if (m_genParams.poleGenInfo.size() == 0)
    {
        warning("Pole generator: pole gen info is empty\n");
        return;
    }

    auto addClusterNodeInstance = [&getOrCreateCluster, &center](
        std::string const& nodeName, Math::Mat4 const& transform)
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

    auto addClusterNodeInstanceFlipped = [&getOrCreateCluster, &center](
        std::string const& nodeName, Math::Mat4 const& transform)
    {
        Math::Mat4 const rotatedTransform(transformTurnaroundZ(transform));

        if (auto cluster = getOrCreateCluster(nodeName,
                /*matInfo=*/RailwayMeshGeneratorConfig::MaterialInfo{}))
        {
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

    createPoles(m_genParams.poleGenInfo, addClusterNodeInstance);
    createAnchors(m_genParams.poleGenInfo, addClusterNodeInstanceFlipped);

    if (m_genParams.generateWires)
    {
        for (auto const& catenarySegment : m_catenarySegments)
        {
            catenarySegment->validateClusters(getOrCreateCluster, center);
        }
    }
}

void
DynamicElectrificationPoleGenerator::loadCatenarySegments()
{
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
    ObjectMessageLogger log("DynamicElectrificationPoleGenerator::loadCatenarySegments");
    ObjectMessageLoggerGuard log_guard(log);
#endif

    int numPoles = m_genParams.poleGenInfo.size();
    assert(numPoles >= 2);

    auto makeGenCatenaryArgs = [this](
        CatenaryContextData const& catenaryData, int poleIdx)
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

    bool nextNonStitched = false;
    std::list<DynamicElectrificationCatenaryGenerator::CreateArgs> genDataList;
    for (int i = 1; i < numPoles; i++)
    {
        auto const& rightPole = m_genParams.poleGenInfo[i];
        auto const& leftPole = m_genParams.poleGenInfo[i - 1];

        DynamicElectrificationCatenaryGenerator::ConnectTwoPolesByCatenaryArgs
            connectArgs{ leftPole, rightPole, pLog };
        connectArgs.leftPolePreset = leftPole.preset;
        connectArgs.rightPolePreset = rightPole.preset;
        connectArgs.defaultCantileverPreset = m_genParams.defaultCantileverPreset;
        connectArgs.creationMode = CreationMode::NODES; // m_genParams.creationMode;

        Math::Vec3 leftPos = leftPole.transform.getTranslate();
        Math::Vec3 rightPos = rightPole.transform.getTranslate();

        if (std::abs((leftPos - rightPos).length()) >=
            ELECTRIFICATION_STITCHED_CATENARY_LENGTH_MIN)
        {
            if (i == 1 &&
                (m_genParams.tensionPathPlacement == TensionPathPlacement::Start ||
                    m_genParams.tensionPathPlacement == TensionPathPlacement::None))
            {
                connectArgs.leftStitch = false;
            }
            else
            {
                if (!nextNonStitched)
                {
                    connectArgs.leftStitch = m_genParams.catenaryType == CatenaryType::Stitched;
                }
                else
                {
                    connectArgs.leftStitch = false;
                }

                nextNonStitched = false;
            }

            if (i == numPoles - 1 &&
                (m_genParams.tensionPathPlacement == TensionPathPlacement::End ||
                    m_genParams.tensionPathPlacement == TensionPathPlacement::None))
            {
                connectArgs.rightStitch = false;
            }
            else
            {
                connectArgs.rightStitch = m_genParams.catenaryType == CatenaryType::Stitched;
            }
        }
        else
        {
            connectArgs.leftStitch = false;
            connectArgs.rightStitch = false;

            auto lastGenData = genDataList.rbegin();
            if (lastGenData != genDataList.rend())
            {
                lastGenData->contextData.rightStitch = false;
            }

            nextNonStitched = true;
        }

        if (m_genParams.middleTensioning == TensioningType::Compensated)
        {
            if (i == m_startAnchorPoleIdx + 1)
            {
                connectArgs.midPointAnchorType = MidPointAnchorType::END;
            }
            else if (i == m_endAnchorPoleIdx)
            {
                connectArgs.midPointAnchorType = MidPointAnchorType::START;
            }
        }
        else if (m_genParams.middleTensioning == TensioningType::SemiCompensated)
        {
            if (i == m_middleAnchorPoleIdx)
            {
                connectArgs.midPointAnchorType = MidPointAnchorType::MIDDLE;
            }
        }

        auto resultList =
            DynamicElectrificationCatenaryGenerator::connectTwoPolesByCatenary(connectArgs);
        if (resultList.size() == 0)
        {
#ifdef DYNAMIC_ELECTRIFICATION_DEBUG
            log.error("Cannot generate catenary between %d and %d poles within tension object #%d\n",
                (i - 1), i, getKey().id);
#endif
            continue;
        }

        genDataList.insert(genDataList.cend(), resultList.cbegin(), resultList.cend());
    }//for (int i = 1; i < m_genParams.poleGenInfo.size(); i++)

    if (m_startAnchorCatenaryData)
    {
        DynamicElectrificationCatenaryGenerator::CreateArgs connectArgs =
            makeGenCatenaryArgs(*m_startAnchorCatenaryData, m_startAnchorPoleIdx);
        assert(m_startAnchorPoleIdx + 1 < m_genParams.poleGenInfo.size());
        genDataList.push_back(connectArgs);
    }

    if (m_endAnchorCatenaryData)
    {
        DynamicElectrificationCatenaryGenerator::CreateArgs connectArgs =
            makeGenCatenaryArgs(*m_endAnchorCatenaryData, m_endAnchorPoleIdx);
        assert(m_endAnchorPoleIdx > 0);
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

void DynamicElectrificationPoleGenerator::loadTensioningMounts()
{
    if (m_genParams.poleGenInfo.size() <= 3)
    {
        warning("Pole generator: size of pole gen info data <= 3\n");
        return;
    }

    assert(m_startAnchorPoleIdx >= 0 && m_startAnchorPoleIdx < m_genParams.poleGenInfo.size());
    assert(m_endAnchorPoleIdx >= 0 && m_endAnchorPoleIdx < m_genParams.poleGenInfo.size());
    assert(m_middleAnchorPoleIdx >= 0 && m_middleAnchorPoleIdx < m_genParams.poleGenInfo.size());

    auto genHost = DynamicElectrification::instance();
    assert(genHost);

    auto getHelperData = [this, &genHost](DynamicPoleGenInfo const& genInfo)
        -> std::optional<PoleSourceNodeHelperData>
    {
        auto chooser = genHost->getOrCreateNodeNameChooser(genInfo.preset);
        if (!chooser)
        {
            error("Pole generator: can not get name chooser for preset: %s\n", genInfo.preset.c_str());
            return std::nullopt;
        }

        auto poleNodeName = chooser->choosePoleNodeName(genInfo);
        if (!poleNodeName)
        {
            error("Pole generator: can not choose pole node path for preset: %s\n", genInfo.preset.c_str());
            return std::nullopt;
        }

        fillPoleSourceNodeHelperData(*poleNodeName);
        return findPoleNodeHelperData(*poleNodeName);
    };

    auto getTensionMountTransform =
        [this, &getHelperData](DynamicPoleGenInfo const& genInfo)
        -> std::optional<Math::Mat4>
    {
        if (auto helperData = getHelperData(genInfo))
        {
            if (helperData->tensioningMountPoint)
            {
                return genInfo.transform * (*helperData->tensioningMountPoint);
            }
        }

        error("Pole generator: can not get helper data\n");

        return std::nullopt;
    };

    auto getConnectorTransform =
        [this, &getHelperData](DynamicPoleGenInfo const& genInfo)
        -> std::optional<Math::Mat4>
    {
        auto helperData = getHelperData(genInfo);
        if (!helperData)
        {
            error("Pole generator: can not get helper data\n");

            return std::nullopt;
        }

        auto findIt = helperData->connectorByCantilevers.find(POLE_CONNECTOR_ID_NT_WS);
        if (findIt != helperData->connectorByCantilevers.cend())
        {
            error("Pole generator: can not get connectors data\n");

            auto const& cantilever = findIt->second;

            return genInfo.transform * cantilever.cable;
        }

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
        Math::Vec3 rotationAxis = Math::cross(directionTo, directionFrom);
        Math::quat rotationQuat{ Math::vec3{ rotationAxis }, Math::abs(angleDegrees) };
        Math::Mat4 rotationTransform{ rotationQuat };

        return mountTransform * rotationTransform;
    };

    auto startMountTransform = getTensionMountTransform(m_genParams.poleGenInfo[m_startAnchorPoleIdx]);
    auto endMountTransform = getTensionMountTransform(m_genParams.poleGenInfo[m_endAnchorPoleIdx]);
    auto middlePoleConnector = getConnectorTransform(m_genParams.poleGenInfo[m_middleAnchorPoleIdx]);

    if (startMountTransform && middlePoleConnector)
    {
        m_startTensioningMount = getRotatedMountTransform(
            *startMountTransform, *middlePoleConnector);
    }

    if (endMountTransform && middlePoleConnector)
    {
        m_endTensioningMount = getRotatedMountTransform(
            *endMountTransform, *middlePoleConnector);
    }
}

void DynamicElectrificationPoleGenerator::loadAnchorBlocksCatenaryData()
{
    if (m_genParams.poleGenInfo.size() <= 3)
    {
        warning("Pole generator: size of pole gen info data <= 3\n");
        return;
    }

    assert(m_startAnchorPoleIdx >= 0 && m_startAnchorPoleIdx < m_genParams.poleGenInfo.size());
    assert(m_endAnchorPoleIdx >= 0 && m_endAnchorPoleIdx < m_genParams.poleGenInfo.size());
    assert(m_middleAnchorPoleIdx >= 0 && m_middleAnchorPoleIdx < m_genParams.poleGenInfo.size());

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto getNodeHelperData = [this, &generatorsHost]
    (DynamicPoleGenInfo const& genInfo, std::optional<TensioningType> tensioningType = std::nullopt)
        -> std::optional<PoleSourceNodeHelperData>
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(genInfo.preset);
        if (!nodeNameChooser)
        {
            error("Pole generator: can not get name chooser for preset: %s\n", genInfo.preset.c_str());
            return std::nullopt;
        }

        std::optional<std::string> nodePath;
        if (tensioningType)
        {
            AnchorContextData anchorData{*tensioningType, /*middleTensioning=*/true};
            nodePath = nodeNameChooser->chooseAnchorBlockNodeName(anchorData);
        }
        else
        {
            nodePath = nodeNameChooser->choosePoleNodeName(genInfo);
        }

        if (nodePath)
        {
            fillPoleSourceNodeHelperData(*nodePath);
            return findPoleNodeHelperData(*nodePath);
        }
        else
        {
            error("Pole generator: can not choose pole node path for preset: %s\n", genInfo.preset.c_str());
        }

        return std::nullopt;
    };

    auto findCantileverWithIDs = [this](PoleSourceNodeHelperData const& helperData,
        std::list<std::string> const& idList)
        -> std::optional<PoleCantileverConnectors>
    {
        std::stringstream ss;

        for (std::string const& id : idList)
        {
            ss << id << ", ";

            auto findIt = helperData.connectorByCantilevers.find(id);
            if (findIt != helperData.connectorByCantilevers.cend())
            {
                return findIt->second;
            }
        }

        {
            error("Pole generator: can not get connector's data with these ID: [%s]\n", ss.str().c_str());
        }

        return std::nullopt;
    };

    auto getAnchorCatenaryData =
        [this, &getNodeHelperData, &findCantileverWithIDs, &generatorsHost]
    (DynamicPoleGenInfo const& anchorPoleGenInfo,
        DynamicPoleGenInfo const& transitionalPoleGenInfo,
        TensioningType tensioningType, Math::Mat4 const tensioningMount)
        -> std::optional<CatenaryContextData>
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(anchorPoleGenInfo.preset);
        if (!nodeNameChooser)
        {
            error("Pole generator: cannot get name chooser for preset: %s\n", anchorPoleGenInfo.preset.c_str());
            return std::nullopt;
        }

        auto anchorBlockData = getNodeHelperData(anchorPoleGenInfo, tensioningType);
        assert(anchorBlockData);
        auto transitionalPoleData = getNodeHelperData(transitionalPoleGenInfo);
        assert(transitionalPoleData);

        auto anchorCantilever = findCantileverWithIDs(
            *anchorBlockData, std::list<std::string>{ POLE_CONNECTOR_ID_NT_WC });
        assert(anchorCantilever);
        auto transitionalPoleCantilever = findCantileverWithIDs(
            *transitionalPoleData, std::list<std::string>{ POLE_CONNECTOR_ID_NT_WS });
        assert(transitionalPoleCantilever);

        bool transitionalStitched = m_genParams.catenaryType == CatenaryType::Stitched;

        CatenaryContextData contextData =
            DynamicElectrificationCatenaryGenerator::connectTwoCantileversByCatenary(
                DynamicElectrificationCatenaryGenerator::ConnectTwoCantileversArgs{
                    *transitionalPoleCantilever, *anchorCantilever,
                    transitionalPoleGenInfo.transform, tensioningMount,
                    transitionalStitched, /*rightStitch=*/false});

        auto writeNodeName =
            [&nodeNameChooser](CatenaryAttachableType attachableType,
                CatenaryAttachableLength attachableLengthType,
                CatenaryAttachableNodeName& nodeNameOutput)
        {
            if (auto name = nodeNameChooser->chooseCatenaryAttachableNodeName(attachableType,
                    attachableLengthType))
            {
                nodeNameOutput[attachableLengthType] = *name;
            }
        };

        auto writeNodes = [&nodeNameChooser, &writeNodeName](
                              CatenaryAttachableType attachableType,
                              CatenaryAttachableNodeName& nodeNameOutput)
        {
            writeNodeName(attachableType, CatenaryAttachableLength::Short,
                nodeNameOutput);
            writeNodeName(attachableType, CatenaryAttachableLength::Medium,
                nodeNameOutput);
            writeNodeName(attachableType, CatenaryAttachableLength::Long,
                nodeNameOutput);
        };

        writeNodes(CatenaryAttachableType::Dropper, contextData.dropperNodeName);
        writeNodes(CatenaryAttachableType::Dropper1w, contextData.dropperNodeName1w);
        writeNodes(CatenaryAttachableType::InsulatorDropper,
            contextData.insulatorDropperNodeName);
        writeNodes(CatenaryAttachableType::CompensatedMidpointAnchor,
            contextData.compMidPointAnchorName);
        writeNodes(CatenaryAttachableType::SemiCompensatedMidpointAnchor,
            contextData.scompMidPointAnchorName);

        contextData.isAnchor = true;

        return contextData;
    };

    if (m_startTensioningMount)
    {
        m_startAnchorCatenaryData = getAnchorCatenaryData(
            m_genParams.poleGenInfo[m_startAnchorPoleIdx], m_genParams.poleGenInfo[m_middleAnchorPoleIdx],
            m_genParams.middleTensioning, *m_startTensioningMount);
    }

    if (m_endTensioningMount)
    {
        m_endAnchorCatenaryData = getAnchorCatenaryData(
            m_genParams.poleGenInfo[m_endAnchorPoleIdx], m_genParams.poleGenInfo[m_middleAnchorPoleIdx],
            m_genParams.middleTensioning, *m_endTensioningMount);
    }
}

void
DynamicElectrificationPoleGenerator::maybeCreateRootNode()
{
    if (!m_rootNode)
    {
        String nodeName = String::format("poles_%d", m_key.id);
        Math::Vec3 rootPosition = getBoundBox().getCenter();

        m_rootNode = NodeDummy::create();
        m_rootNode->setName(nodeName);
        m_rootNode->setWorldPosition(rootPosition);
    }
}

void
DynamicElectrificationPoleGenerator::createPoles(
    std::vector<electrification::DynamicPoleGenInfo> const& poleGenInfo,
    CreateInstanceFunc const& sink) const
{
    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    for (auto const& info : m_genParams.poleGenInfo)
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
        assert(nodeNameChooser);

        std::optional<std::string> poleNodeName = info.isSuspension
            ? nodeNameChooser->choosePortalSuspensionNodeName(info)
            : nodeNameChooser->choosePoleNodeName(info);
        if (!poleNodeName)
        {
            continue;
        }

        sink(*poleNodeName, info.transform);
    }
}

void
DynamicElectrificationPoleGenerator::createAnchors(
    std::vector<electrification::DynamicPoleGenInfo> const& poleGenInfo,
    CreateInstanceFunc const& sink) const
{
    struct AddAnchorInfo
    {
        Math::Mat4 tensioningMount;
        int poleIdx{0};

        AddAnchorInfo(Math::Mat4 const& mountTransform, int idx)
            : tensioningMount(mountTransform)
            , poleIdx(idx)
        {
        }
    };

    std::vector<AddAnchorInfo> endpointAnchors;

    if (m_startTensioningMount)
    {
        assert(m_startAnchorPoleIdx >= 0 && m_startAnchorPoleIdx < m_genParams.poleGenInfo.size());
        endpointAnchors.emplace_back(*m_startTensioningMount, m_startAnchorPoleIdx);
    }

    if (m_endTensioningMount)
    {
        assert(m_endAnchorPoleIdx >= 0 && m_endAnchorPoleIdx < m_genParams.poleGenInfo.size());
        endpointAnchors.emplace_back(*m_endTensioningMount, m_endAnchorPoleIdx);
    }

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    for (auto const& endpointAnchor : endpointAnchors)
    {
        AnchorContextData anchorContextData{
            m_genParams.middleTensioning, /*middleTensioning=*/true};

        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(
            m_genParams.poleGenInfo[endpointAnchor.poleIdx].preset);
        assert(nodeNameChooser);

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
