#include "DynamicElectrificationPortalGenerator.h"

#include "neokon-shared/electrification/DynamicElectrification.h"
#include "neokon-shared/electrification/DynamicElectrificationCatenaryGenerator.h"
#include "neokon-shared/electrification/CatenaryGenerationConstants.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"
#include "neokon-shared/electrification/utils/ElectrificationNodeConnectorsData.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/TransformTurnaroundZ.h"

#include <UnigineMathLib.h>
#include <UnigineWorld.h>

using namespace Unigine;
using namespace electrification;

namespace
{
std::optional<Math::Vec3> getConnectorFromMap(
    ConnectorsMap const& connectorsMap, std::unordered_set<std::string> const& connectorsToFind)
{
    std::optional<Math::Vec3> result;
    for (auto& [connector, transform] : connectorsMap)
    {
        auto searchResult = connectorsToFind.find(connector);
        if (searchResult != connectorsToFind.end())
        {
            result = transform.getTranslate();
            break;
        }
    }

    return result;
}
}

//===================================================================
// DynamicElectrificationPortalGenerator implementation
//===================================================================
DynamicElectrificationPortalGenerator::DynamicElectrificationPortalGenerator(
    uint32_t id, CreateArgs&& args)
    : DynamicElectrificationPart(id, 0)
    , m_genParams(std::move(args))
{
    setBBox(fromGlm(m_genParams.aabb.minCoords()), fromGlm(m_genParams.aabb.maxCoords()));

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);
    for (auto const& info : m_genParams.portalGenInfo)
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
        assert(nodeNameChooser);

        auto config = generatorsHost->getOrCreateMeshConfig(info.preset);
        assert(config);

        if (!config->getPortalData().isCrossSpan)
        {
            auto crossroadResult = nodeNameChooser->choosePortalCrossbarNodeName(info);
            if (crossroadResult)
            {
                fillPoleSourceNodeHelperData(crossroadResult->path);
            }
            else
            {
                error("Portal generator: cannot choose portal crossroad for preset \"%s\", and "
                      "desired length %.2f.\n",
                    info.preset, info.desiredLength);
            }
        }

        auto leftPoleNodePath = nodeNameChooser->chooseLeftPortalPoleNodeName(info);
        if (leftPoleNodePath)
        {
            fillNodeConnectorsData(*leftPoleNodePath);
            fillPoleSourceNodeHelperData(*leftPoleNodePath);
        }
        else
        {
            error("Portal generator: cannot choose left portal for preset \"%s\".\n", info.preset);
        }

        auto rightPoleNodePath = nodeNameChooser->chooseRightPortalPoleNodeName(info);
        if (rightPoleNodePath)
        {
            fillNodeConnectorsData(*rightPoleNodePath);
            fillPoleSourceNodeHelperData(*rightPoleNodePath);
        }
        else
        {
            error("Portal generator: cannot choose right portal for preset \"%s\".\n", info.preset);
        }

        if (config->getPortalData().isCrossSpan)
        {
            fillNodeConnectorsData(config->getPortalData().crossWireInsulatorNode);
            fillNodeConnectorsData(config->getPortalData().dropperClamp);
            fillNodeConnectorsData(config->getPortalData().spanWireInsulatorNode);
        }
    }
}

bool DynamicElectrificationPortalGenerator::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    if (old == nullptr)
    {
        return true;
    }

    auto otherPortalGenerator = std::dynamic_pointer_cast<DynamicElectrificationPortalGenerator>(old);
    if (!otherPortalGenerator)
    {
        return true;
    }

    return getUniqueGenValue() != otherPortalGenerator->getUniqueGenValue();
}

bool DynamicElectrificationPortalGenerator::load()
{
    return true;
}

void DynamicElectrificationPortalGenerator::unload()
{
    if (m_rootNode)
    {
        m_rootNode.deleteForce();
    }

    DynamicElectrificationPart::unload();
}

Unigine::NodePtr DynamicElectrificationPortalGenerator::createNodes()
{
    auto createWire = [this](DynamicPortalGenInfo const& info,
                          DynamicElectrificationCatenaryGenerator::CreateArgs&& createArgs)
    {
        DynamicElectrificationCatenaryGeneratorPtr wire =
            std::make_shared<DynamicElectrificationCatenaryGenerator>(info.portalId,
                std::move(createArgs));
        if (wire)
        {
            wire->load();
        }

        NodePtr catenaryNode = wire->createNodes();

        if (NodePtr existingNode = m_rootNode->findNode(catenaryNode->getName(),
                /*recursive=*/0))
        {
            existingNode.deleteLater();
        }

        m_rootNode->addChild(catenaryNode);
        catenaryNode->setWorldTransform(wire->globalTransform());

        m_wireGenerators.push_back(wire);
    };

    auto createNode = [this](std::string const& nodePath, Math::Mat4 const& transform)
    {
        if (nodePath.empty())
        {
            // TODO: maybe log error.
            return;
        }

        if (NodePtr childNode = World::loadNode(nodePath.c_str()))
        {
            m_rootNode->addChild(childNode);
            Math::Mat4 baseRotate = Math::Mat4(Math::rotate(childNode->getRotation()));
            childNode->setWorldTransform(transform * baseRotate);
        }
        else
        {
            error("Portal generator: can not load world node: %s.\n", nodePath.c_str());
        }
    };

    if (m_genParams.portalGenInfo.empty())
    {
        error("Portal generator: list of portal generation data is empty.\n");

        return nullptr;
    }

    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    maybeCreateRootNode();

    for (auto& wireGenerator : m_wireGenerators)
    {
        if (auto wireNode = wireGenerator->rootNode())
        {
            wireNode.deleteLater();
        }
    }
    m_wireGenerators.clear();

    bool const fillClusterData = m_portalCreatesForClusters.empty();
    bool const fillClusterDataFlex = m_crossSpanCreatesForClusters.empty();

    for (auto const& info : m_genParams.portalGenInfo)
    {
        auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
        assert(nodeNameChooser);

        auto config = generatorsHost->getOrCreateMeshConfig(info.preset);
        assert(config);

        if (config->getPortalData().isCrossSpan)
        {
            auto portalCreateData = prepareCrossSpan(info);
            if (!portalCreateData)
            {
                // Skip this portal, error message has been written earlier.
                continue;
            }

            if (m_genParams.creationMode == electrification::CreationMode::NODES)
            {
                portalCreateData->listNodes(createNode);
            }
            else if (fillClusterDataFlex)
            {
                m_crossSpanCreatesForClusters.emplace_back(*portalCreateData);
            }

            if (portalCreateData->catenaryArgs)
            {
                createWire(info, std::move(*portalCreateData->catenaryArgs));
            }

            if (portalCreateData->spanWireArgs)
            {
                createWire(info, std::move(*portalCreateData->spanWireArgs));
            }
        }
        else
        {
            auto portalCreateData = preparePortal(info);
            if (!portalCreateData)
            {
                // Skip this portal, error message has been written earlier.
                continue;
            }

            if (portalCreateData->spanWireArgs)
            {
                if (portalCreateData->spanWireArgs)
                {
                    createWire(info, std::move(*portalCreateData->spanWireArgs));
                }
            }

            if (m_genParams.creationMode == electrification::CreationMode::NODES)
            {
                portalCreateData->listNodes(createNode);
            }
            else if (fillClusterData)
            {
                m_portalCreatesForClusters.emplace_back(*portalCreateData);
            }
        }
    }

    return m_rootNode;
}

void
DynamicElectrificationPortalGenerator::validateClusters(
    GetOrCreateClusterFunc const& getOrCreateCluster,
    std::optional<Unigine::Math::Vec3> const& center) const
{
    if (m_genParams.creationMode != electrification::CreationMode::CLUSTER)
    {
        return;
    }

    if (m_genParams.portalGenInfo.empty())
    {
        return;
    }

    auto addClusterNodeInstance = [this, &getOrCreateCluster, &center](
                                      std::string const& nodeName, Math::Mat4 const& transform)
    {
        if (auto cluster = getOrCreateCluster(nodeName, RailwayMeshGeneratorConfig::MaterialInfo{}))
        {
            // TODO: Investigate, why do we need such transformation.
            Math::Mat4 const rotatedTransform = transformTurnaroundZ(transform);

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
            const Math::Mat4 localClusterTransform = invClusterTransform * transform;
            const int instanceIdx = cluster->addMeshTransform();
            cluster->setMeshTransform(instanceIdx, Math::mat4{localClusterTransform});
        }
    };

    for (auto const& portalCreateData : m_portalCreatesForClusters)
    {
        portalCreateData.listNodes(addClusterNodeInstance);
    }

    for (auto const& portalCreateData : m_crossSpanCreatesForClusters)
    {
        portalCreateData.listNodes(addClusterNodeInstance);
    }

    for (auto& wireGenerator : m_wireGenerators)
    {
        wireGenerator->validateClusters(getOrCreateCluster, center);
    }
}

void
DynamicElectrificationPortalGenerator::maybeCreateRootNode()
{
    if (!m_rootNode)
    {
        String nodeName = String::format("portals_%u", m_key.id);
        Math::Vec3 rootPosition = getBoundBox().getCenter();

        m_rootNode = NodeDummy::create();
        m_rootNode->setName(nodeName);
        m_rootNode->setWorldPosition(rootPosition);
    }
}

std::optional<DynamicElectrificationPortalGenerator::CreatePortalData>
DynamicElectrificationPortalGenerator::preparePortal(
    electrification::DynamicPortalGenInfo const& info) const
{
    // TODO: use static method in the appropriate Part instead of whole DynamicElectrification.
    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
    assert(nodeNameChooser);

    auto config = generatorsHost->getOrCreateMeshConfig(info.preset);
    assert(config);

    auto crossroadResult = nodeNameChooser->choosePortalCrossbarNodeName(info);
    if (!crossroadResult)
    {
        error("Portal generator: cannot choose portal crossroad for preset \"%s\", and desired "
              "length %.2f.\n",
            info.preset, info.desiredLength);
        return std::nullopt;
    }

    std::optional<Math::Mat4> crossbarMountPointLeft;
    std::optional<Math::Mat4> crossbarMountPointRight;

    if (auto crossbarHelperData = findPoleNodeHelperData(crossroadResult->path))
    {
        crossbarMountPointLeft = crossbarHelperData->crossbarMountPointLeft;
        crossbarMountPointRight = crossbarHelperData->crossbarMountPointRight;
    }

    auto leftPoleNodePath = nodeNameChooser->chooseLeftPortalPoleNodeName(info);
    if (!leftPoleNodePath)
    {
        error("Portal generator: cannot choose left portal for preset \"%s\".\n", info.preset);
        return std::nullopt;
    }

    auto rightPoleNodePath = nodeNameChooser->chooseRightPortalPoleNodeName(info);
    if (!rightPoleNodePath)
    {
        error("Portal generator: cannot choose right portal for preset \"%s\".\n", info.preset);
        return std::nullopt;
    }

    auto leftPoleHelperData = findPoleNodeHelperData(*leftPoleNodePath);
    auto rightPoleHelperData = findPoleNodeHelperData(*rightPoleNodePath);
    if (!leftPoleHelperData || !rightPoleHelperData)
    {
        return std::nullopt;
    }

    Math::Mat4 const& rootTransform = info.transform;

    // Rigel.
    const double rigelZShift = config->getPortalData().crossBarZShiftM;

    Math::Mat4 rigelTransform;
    rigelTransform.setTranslate(Math::Vec3{0.0, 0.0, rigelZShift});

    // Poles.
    Math::Mat4 translationMat;
    Math::Mat4 leftPoleTransform;
    if (!crossbarMountPointLeft)
    {
        message("Portal generator: cannot find dummy (left, pole1) for crossbar "
                "node \"%s\", object id: %d",
            crossroadResult->path, m_key.id);

        const double leftPoleOffset = -crossroadResult->actualLength * 0.5;
        translationMat.setIdentity();
        translationMat.setTranslate(Math::Vec3{0.0, leftPoleOffset, 0.0});

        leftPoleTransform = transformTurnaroundZ(translationMat);
    }
    else
    {
        leftPoleTransform = *crossbarMountPointLeft;
    }

    Math::Mat4 rightPoleTransform;
    if (!crossbarMountPointRight)
    {
        message("Portal generator: cannot find dummy (right, pole2) for crossbar "
                "node \"%s\", object id: %d",
            crossroadResult->path, m_key.id);

        const double rightPoleOffset = crossroadResult->actualLength * 0.5;
        translationMat.setIdentity();
        translationMat.setTranslate(Math::Vec3{0.0, rightPoleOffset, 0.0});

        rightPoleTransform = translationMat;
    }
    else
    {
        rightPoleTransform = *crossbarMountPointRight;
    }

    Math::Mat4 const leftPoleWorldTransform = rootTransform * leftPoleTransform;
    Math::Mat4 const rightPoleWorldTransform = rootTransform * rightPoleTransform;

    ConnectorsMap leftPoleConnectors = findNodeConnectorsData(*leftPoleNodePath);
    ConnectorsMap rightPoleConnectors = findNodeConnectorsData(*rightPoleNodePath);

    std::optional<Math::Vec3> leftConnector =
        getConnectorFromMap(leftPoleConnectors, {POLE_CONNECTOR_ID_FT});
    std::optional<Math::Vec3> rightConnector =
        getConnectorFromMap(rightPoleConnectors, {POLE_CONNECTOR_ID_FT});

    if (!leftConnector || !rightConnector)
    {
        return CreatePortalData{
            NodePathWithTransform{crossroadResult->path, rootTransform * rigelTransform},
            NodePathWithTransform{*leftPoleNodePath, leftPoleWorldTransform},
            NodePathWithTransform{*rightPoleNodePath, rightPoleWorldTransform}, std::nullopt};
    }

    DynamicElectrificationCatenaryGenerator::CreateArgs spanWireArgs;
    spanWireArgs.creationMode = m_genParams.creationMode;
    spanWireArgs.defaultCantileverPreset = info.preset;

    auto leftPoleInitialTransform = getOrLoadInitialNodeTransform(*leftPoleNodePath);
    auto rightPoleInitialTransform = getOrLoadInitialNodeTransform(*rightPoleNodePath);
    if (!leftPoleInitialTransform || !rightPoleInitialTransform)
    {
        return std::nullopt;
    }

    spanWireArgs.contextData.messengerWire.startPosition =
        leftPoleWorldTransform * (*leftPoleInitialTransform * *leftConnector);
    spanWireArgs.contextData.messengerWire.endPosition =
        rightPoleWorldTransform * (*rightPoleInitialTransform * *rightConnector);

    spanWireArgs.spanWireSuspensionDistances = info.suspensionDistances;
    spanWireArgs.isSpanWire = true;

    return CreatePortalData{
        NodePathWithTransform{crossroadResult->path, rootTransform * rigelTransform},
        NodePathWithTransform{*leftPoleNodePath, leftPoleWorldTransform},
        NodePathWithTransform{*rightPoleNodePath, rightPoleWorldTransform}, spanWireArgs
    };
}

std::optional<DynamicElectrificationPortalGenerator::CreateCrossSpanData>
DynamicElectrificationPortalGenerator::prepareCrossSpan(
    electrification::DynamicPortalGenInfo const& info) const
{
    // TODO: use static method in the appropriate Part instead of whole DynamicElectrification.
    auto generatorsHost = DynamicElectrification::instance();
    assert(generatorsHost);

    auto nodeNameChooser = generatorsHost->getOrCreateNodeNameChooser(info.preset);
    assert(nodeNameChooser);

    auto config = generatorsHost->getOrCreateMeshConfig(info.preset);
    assert(config);

    auto leftPoleNodePath = nodeNameChooser->chooseLeftPortalPoleNodeName(info);
    if (!leftPoleNodePath)
    {
        error("Portal generator: cannot choose left portal for preset \"%s\".\n", info.preset);
        return std::nullopt;
    }

    auto rightPoleNodePath = nodeNameChooser->chooseRightPortalPoleNodeName(info);
    if (!rightPoleNodePath)
    {
        error("Portal generator: cannot choose right portal for preset \"%s\".\n", info.preset);
        return std::nullopt;
    }

    Math::Mat4 const& rootTransform = info.transform;

    // Poles.
    const double leftPoleOffset = -info.desiredLength * 0.5;
    const double rightPoleOffset = info.desiredLength * 0.5;

    Math::Mat4 translationMat;
    translationMat.setIdentity();
    translationMat.setTranslate(Math::Vec3{0.0, leftPoleOffset, 0.0});

    Math::Mat4 leftPoleTransform = transformTurnaroundZ(translationMat);

    translationMat.setIdentity();
    translationMat.setTranslate(Math::Vec3{0.0, rightPoleOffset, 0.0});

    Math::Mat4 rightPoleTransform = translationMat;

    Math::Mat4 const leftPoleWorldTransform = rootTransform * leftPoleTransform;
    Math::Mat4 const rightPoleWorldTransform = rootTransform * rightPoleTransform;

    // Insulators
    ConnectorsMap leftPoleConnectors = findNodeConnectorsData(*leftPoleNodePath);
    ConnectorsMap rightPoleConnectors = findNodeConnectorsData(*rightPoleNodePath);

    std::optional<Math::Vec3> leftNtConnector, rightNtConnector;
    std::optional<Math::Vec3> leftKpConnector, rightKpConnector;
    std::optional<Math::Vec3> leftFtConnector, rightFtConnector;

    leftNtConnector = getConnectorFromMap(leftPoleConnectors, {POLE_CONNECTOR_ID_CROSS_INS});
    rightNtConnector = getConnectorFromMap(rightPoleConnectors, {POLE_CONNECTOR_ID_CROSS_INS});

    leftKpConnector = getConnectorFromMap(leftPoleConnectors, {POLE_CONNECTOR_ID_TFT});
    leftFtConnector = getConnectorFromMap(leftPoleConnectors, {POLE_CONNECTOR_ID_BFT});
    rightKpConnector = getConnectorFromMap(rightPoleConnectors, {POLE_CONNECTOR_ID_TFT});
    rightFtConnector = getConnectorFromMap(rightPoleConnectors, {POLE_CONNECTOR_ID_BFT});

    if (!leftNtConnector || !rightNtConnector || !leftKpConnector || !leftFtConnector
        || !rightKpConnector || !rightFtConnector)
    {
        return CreateCrossSpanData{
            NodePathWithTransform{*leftPoleNodePath, leftPoleWorldTransform},
            NodePathWithTransform{*rightPoleNodePath, rightPoleWorldTransform},
            std::nullopt, std::nullopt};
    }

    // Catenary
    DynamicElectrificationCatenaryGenerator::CreateArgs catenaryArgs;
    catenaryArgs.creationMode = CreationMode::NODES;
    catenaryArgs.defaultCantileverPreset = info.preset;
    catenaryArgs.isCrossSpan = true;
    catenaryArgs.dropperPositions = info.suspensionDropperConnectors;

    auto leftPoleInitialTransform = getOrLoadInitialNodeTransform(*leftPoleNodePath);
    auto rightPoleInitialTransform = getOrLoadInitialNodeTransform(*rightPoleNodePath);
    if (!leftPoleInitialTransform || !rightPoleInitialTransform)
    {
        return std::nullopt;
    }

    catenaryArgs.contextData.messengerWire.startPosition =
        leftPoleWorldTransform * (*leftPoleInitialTransform * *leftNtConnector);
    catenaryArgs.contextData.messengerWire.endPosition =
        rightPoleWorldTransform * (*leftPoleInitialTransform * *rightNtConnector);

    electrification::CatenaryWireData wireData;
    wireData.startPosition = leftPoleWorldTransform *
        (*leftPoleInitialTransform * *leftKpConnector);
    wireData.endPosition = rightPoleWorldTransform *
        (*rightPoleInitialTransform * *rightKpConnector);

    catenaryArgs.contextData.contactWires.emplace(ELECTRIFICATION_POLE_WIRE_CONNECTOR_NEAR_INDEX,
        std::move(wireData));

    //TODO: этот блок используется в нескольких местах, сделать общую функцию
    auto writeNodeName = [&nodeNameChooser](CatenaryAttachableType attachableType,
                             CatenaryAttachableLength attachableLengthType,
                             CatenaryAttachableNodeName& nodeNameOutput)
    {
        if (auto name = nodeNameChooser->chooseCatenaryAttachableNodeName(attachableType,
                attachableLengthType))
        {
            nodeNameOutput[attachableLengthType] = *name;
        }
    };

    auto writeNodes = [&nodeNameChooser, &writeNodeName](CatenaryAttachableType attachableType,
                          CatenaryAttachableNodeName& nodeNameOutput)
    {
        writeNodeName(attachableType, CatenaryAttachableLength::Short, nodeNameOutput);
        writeNodeName(attachableType, CatenaryAttachableLength::Medium, nodeNameOutput);
        writeNodeName(attachableType, CatenaryAttachableLength::Long, nodeNameOutput);
    };

    writeNodes(CatenaryAttachableType::SpanWireConnector, catenaryArgs.contextData.spanWireConnectorNodeName);

    // Registration arm wire
    DynamicElectrificationCatenaryGenerator::CreateArgs spanWireArgs;
    spanWireArgs.creationMode = m_genParams.creationMode;
    spanWireArgs.defaultCantileverPreset = info.preset;
    spanWireArgs.isSpanWire = true;
    spanWireArgs.spanWireSuspensionDistances = info.suspensionDistances;

    spanWireArgs.contextData.messengerWire.startPosition = leftPoleWorldTransform *
        (*leftPoleInitialTransform * *leftFtConnector);
    spanWireArgs.contextData.messengerWire.endPosition = rightPoleWorldTransform *
        (*rightPoleInitialTransform * *leftFtConnector);

    return CreateCrossSpanData{
        NodePathWithTransform{*leftPoleNodePath, leftPoleWorldTransform},
        NodePathWithTransform{*rightPoleNodePath, rightPoleWorldTransform},
        catenaryArgs, spanWireArgs};
}
