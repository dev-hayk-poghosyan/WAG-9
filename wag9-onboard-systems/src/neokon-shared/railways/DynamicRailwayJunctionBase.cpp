#include "DynamicRailwayJunctionBase.h"

#include "neokon-shared/GenerateEnoughSplinePoints.h"
#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/Math/WrapValueAround.h"
#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/NormalizedAngleFromTo.h"
#include "neokon-shared/railways/RailSwitchInfoComponents.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConstants.h"
#include "neokon-shared/StringFormat.h"
#include "neokon-shared/TransformFromPositionAndAxis.h"
#include "neokon-shared/TransformTurnaroundZ.h"

#include <UnigineEngine.h>
using namespace Unigine;

#include <glm/gtx/vector_angle.hpp>

#include <random>
#include <sstream>
#include <regex>

namespace
{

const char* SkinnedFrogRail2InBone = "Rail2_In_Dummy";
const char* SkinnedFrogRail2OutBone = "Rail2_Out_Dummy";
const char* SkinnedFrogRail1InBone = "Rail1_In_Dummy";
const char* SkinnedFrogRail1OutBone = "Rail1_Out_Dummy";
const char* SkinnedFrogGrailBeginBone = "Grail0_Dummy";
const char* SkinnedFrogGrailEndBone = "Grail1_Dummy";

const char* FrogGrailPointBegin = "grail0";
const char* FrogGrailPointEnd = "grail1";

const char* GuardBeginNodeName = "BeginPart";
const char* GuardEndNodeName = "EndPart";
const char* GuardMiddleNodeName = "MiddlePart";

const char* PointRailSlicePointBoneName = "cutoff";

const double MinimalSplineProfileOffset2 = 0.005;

Math::Mat4
calcNodeScale(MetaUnigine::NodePtr const& node, Math::Vec3 const& outPosition,
    Math::Vec3 const& inPosition)
{
    double realSize = 1.0;

    auto taga0 = node->findNode("taga0");
    auto taga1 = node->findNode("taga1");
    if (taga0 && taga1)
    {
        realSize = std::abs(taga1->getPosition().y - taga0->getPosition().y);
    }
    else
    {
        WorldBoundBox wbb = node->getBoundBox();
        Math::Vec3 extents(wbb.getMax() - wbb.getMin());
        realSize = extents.y;
    }

    double scaledSize = (outPosition - inPosition).length();
    double scaleFactor = 1.0;
    if (realSize > 0.0)
    {
        scaleFactor = scaledSize / realSize;
    }

    Math::Mat4 scaleMat;
    scaleMat.setScale(Math::Vec3(1., scaleFactor, 1.));
    return scaleMat;
}

template<typename ParentType>
String&
makeChildNameUniqueIndexed(ParentType parent, String& childName)
{
    int index = 0;

    String bakName(childName);
    while (parent->findChild(childName) >= 0)
    {
        childName = String::format("%s%d", bakName.get(), index);
        index++;
    }

    return childName;
}

}//anonymous namespace

DynamicRailwayJunctionBase::HeelBlockInfo::HeelBlockInfo(double start, double end)
{
    this->start = start;
    this->end = end;
}

DynamicRailwayJunctionBase::DynamicRailwayJunctionBase(uint32_t partId,
    const std::string& configPath, std::vector<BezierSpline>&& splines,
    std::vector<SplineNeighborConfigs>&& neighborConfigs,
    const Unigine::Math::Mat4& transform)
    : DynamicRailwayPartGenerated(partId, 0, configPath)
    , m_splines(std::move(splines))
    , m_neighborConfigs(neighborConfigs)
    , m_transform(transform)
{
    assert(!m_splines.empty());

    AlignedBox<glm::dvec3> aabb = m_splines[0].getAABB();

    size_t const totalSplines = m_splines.size();
    for (size_t splineIdx = 1; splineIdx < totalSplines; ++splineIdx)
    {
        aabb |= m_splines[splineIdx].getAABB();
    }

    setBBox(fromGlm(aabb.minCoords()), fromGlm(aabb.maxCoords()));
    m_bbox.setTransform(m_transform);
}

void DynamicRailwayJunctionBase::unload()
{
    m_object = nullptr;
    m_clusters.clear();
    onUnload();
}

NodePtr DynamicRailwayJunctionBase::createNodes()
{
    if (!getMetaObject())
    {
        return NodePtr();
    }
    auto result = getMetaObject()->createUnigineTree();
    result->setName(String::format("junction_%d", m_key.id));
    applyMaterialTextures(result);

    for (const auto& childNodeMatInfo : m_setChildNodeMaterialInfo)
    {
        NodePtr parentNode = result;
        if (!childNodeMatInfo.parentName.empty())
        {
            parentNode = result->findNode(childNodeMatInfo.parentName.data());
            if (!parentNode)
            {
                continue;
            }
        }

        auto childNode = parentNode->findNode(childNodeMatInfo.nodeName.data());
        if (!childNode)
        {
            continue;
        }
        auto mat = childNodeMatInfo.materialInfo.getOrCreateMaterial();
        if (!mat)
        {
            continue;
        }

        auto skinnedNode = checked_ptr_cast<ObjectMeshSkinned>(childNode);
        if (skinnedNode)
        {
            skinnedNode->setMaterial(mat, childNodeMatInfo.surfaceIdx);
            continue;
        }

        auto staticNode = checked_ptr_cast<ObjectMeshStatic>(childNode);
        if (staticNode)
        {
            staticNode->setMaterial(mat, childNodeMatInfo.surfaceIdx);
            continue;
        }
    }

    for (const auto& childNodeInfo : m_createChildNodesInfo)
    {
        auto childNode = World::loadNode(childNodeInfo.nodeFileName.data());
        if (!childNode)
        {
            error("Can not load child node: %s\n", childNodeInfo.nodeFileName.data());
            return NodePtr();
        }
        auto childObject = checked_ptr_cast<ObjectMeshStatic>(childNode);

        childObject->setParent(result);
        childObject->setTransform(childNodeInfo.transform);
        childObject->setName(childNodeInfo.nodeName.data());
        childObject->setMaterial(childNodeInfo.materialInfo.getOrCreateMaterial(), 0);
    }
    m_createChildNodesInfo.clear();

    onGenerationFinished(result);

#ifdef DYNAMIC_RAILWAYS_DEBUG
    if (m_debugVisualizer)
    {
        m_debugVisualizer->visualize(m_transform);
    }
#endif

    return result;
}

void DynamicRailwayJunctionBase::validateClusters(const GetOrCreateClusterFunc& getOrCreateCluster,
    const std::optional<Math::Vec3>& center) const
{
    if (!isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::JunctionClusters))
    {
        return;
    }

    for (const auto& clusterInfo : m_clusters)
    {
        if (auto cluster = getOrCreateCluster(clusterInfo->nodePath, clusterInfo->materialInfo))
        {
            const float visDistance = cluster->getVisibleDistance() * 2.0f; // For fast moving * 2.0f

            const auto invClusterTransform = Math::inverse(cluster->getWorldTransform());
            for (const auto& transform : clusterInfo->transforms)
            {
                if (!center || (*center - transform.getTranslate()).length() < visDistance)
                {
                    cluster->setMeshTransform(cluster->addMeshTransform(),
                        Math::mat4(invClusterTransform * transform));
                }
            }
        }
    }
}

DynamicRailwayPartWithSleepers::ConstSleepersPtr
DynamicRailwayJunctionBase::getSleepers(int pathIdx) const
{
    std::unique_lock<std::mutex> lock(m_sleepersInfosProtect);
    if (m_sleepersInfos.size() <= pathIdx)
    {
        return nullptr;
    }
    return m_sleepersInfos[pathIdx];
}

void
DynamicRailwayJunctionBase::setSleepers(
    std::vector<DynamicRailwayPartWithSleepers::SleepersPtr>&& sleepers)
{
    std::unique_lock<std::mutex> lock(m_sleepersInfosProtect);
    m_sleepersInfos = std::move(sleepers);
}

Math::Mat4
DynamicRailwayJunctionBase::addRandomTransform(SharedDataArgsBase& sharedData, double randomAngle,
    const Math::Mat4& splinePose)
{
    if (!sharedData.random)
    {
        sharedData.random.reset(new std::mt19937(getKey().id + getKey().subId));
    }

    std::uniform_int_distribution<int> flipAngle(0, 1);
    std::uniform_real_distribution<double> addAngle(-randomAngle, randomAngle);

    double angle = addAngle(*sharedData.random);
    if (flipAngle(*sharedData.random) == 1)
    {
        angle += 180.;
    }

    Math::Mat4 addRotate;
    addRotate.setRotate(Math::Vec3::UP, angle);
    return splinePose * addRotate;
}

const DynamicRailwayJunctionBase::ClusterInfoPtr&
DynamicRailwayJunctionBase::getOrCreateClusterInfo(const std::string& sourceModelPath,
    const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo, const std::string& name,
    double visibleDistance)
{
    auto clusterIt = std::find_if(m_clusters.begin(), m_clusters.end(),
        [&sourceModelPath, &materialInfo](const ClusterInfoPtr& value) -> bool
        {
            return (value->nodePath == sourceModelPath) && (value->materialInfo == materialInfo);
        }
    );
    if (clusterIt != m_clusters.end())
    {
        return *clusterIt;
    }

    auto& added = m_clusters.emplace_back(std::make_shared<ClusterInfo>());
    added->nodePath = sourceModelPath;
    added->materialInfo = materialInfo;
    added->visibleDistance = visibleDistance;
    added->name = name;
    return added;
}

const BezierSpline&
DynamicRailwayJunctionBase::getSpline(const SharedDataArgsBase& sharedData, int idx) const
{
    if (m_splines.size() > idx)
    {
        return m_splines[idx];
    }
    auto splineIt = sharedData.splineById.find(idx);
    if (splineIt == sharedData.splineById.end())
    {
        throw std::out_of_range(stringFormat("getSpline(%d) failed, build spline first", idx));
    }
    return splineIt->second;
}

void
DynamicRailwayJunctionBase::setSpline(SharedDataArgsBase& sharedData, int idx,
    BezierSpline&& newSpline)
{
    if (m_splines.size() > idx)
    {
        m_splines[idx] = std::move(newSpline);
    }
    else
    {
        sharedData.splineById[idx] = std::move(newSpline);
    }
}

const TransformSpline&
DynamicRailwayJunctionBase::getSplineSearch(const SharedDataArgsBase& sharedData, int idx) const
{
    auto searchIt = sharedData.splineSearchById.find(idx);
    if (searchIt == sharedData.splineSearchById.end())
    {
        throw std::out_of_range(stringFormat("getSplineSearch(%d) failed, build search first", idx));
    }
    return searchIt->second;
}

std::optional<int>
DynamicRailwayJunctionBase::getOppositeSplineId(const SharedDataArgsBase& sharedData,
    int splineId) const
{
    auto oppositeIt = sharedData.oppositeSplineIds.find(splineId);
    if (oppositeIt == sharedData.oppositeSplineIds.end())
    {
        return std::nullopt;
    }
    return oppositeIt->second;
}

std::optional<int>
DynamicRailwayJunctionBase::getSrcPathIdx(const SharedDataArgsBase& sharedData, int splineId) const
{
    auto pathIt = sharedData.sourcePathSplineIds.find(splineId);
    if (pathIt == sharedData.sourcePathSplineIds.end())
    {
        return std::nullopt;
    }
    return pathIt->second;
}

// Generation parts

void
DynamicRailwayJunctionBase::generateCaps(const Unigine::String& name,
    const RailwayMeshGeneratorConfig::SurfaceGenerationData& config,
    const SplineNeighborConfigs& neighborConfigs, BezierSpline& spline)
{
    if (config.capNodePath.empty())
    {
        return;
    }

    auto addCap = [this, &config](const Math::Mat4& transform, const String& name)
    {
        m_createChildNodesInfo.emplace_back(CreateChildNodeData{name, config.capNodePath,
            transform, config.defaultMaterialInfo});
    };

    const auto& prevConfigPath = neighborConfigs.previous;
    auto prevConfig =
        prevConfigPath ? getOrLoadRailwayMeshGeneratorConfig(*prevConfigPath) : nullptr;
    if (!prevConfig || hasDifferentEmbankmentProfile(prevConfig))
    {
        if (auto transform = spline.calcSplineMatrixAtDistance(0.0, /*clampDistance=*/false,
                TransformOrthogonalization::Off))
        {
            addCap(*transform * Math::Mat4(Math::quat(0., 0., 180.)),
                name + "Front");
        }
    }

    const auto& nextConfigPath = neighborConfigs.next;
    auto nextConfig =
        nextConfigPath ? getOrLoadRailwayMeshGeneratorConfig(*nextConfigPath) : nullptr;
    if (!nextConfig || hasDifferentEmbankmentProfile(nextConfig))
    {
        if (auto transform = spline.calcSplineMatrixAtDistance(spline.arcLength(),
                /*clampDistance=*/false, TransformOrthogonalization::Off))
        {
            addCap(*transform, name + "Back");
        }
    }
}

void DynamicRailwayJunctionBase::generateEmbankment(SharedDataArgsBase& sharedData)
{
    message("generating embankment...\n");

    if (!isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::JunctionEmbankment))
    {
        return;
    }

    std::reference_wrapper<RailwayMeshGeneratorConfig::SurfaceGenerationData> embankmentConfig =
        m_config->getSurfaceData(
            +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentJunctions);
    if (!embankmentConfig.get().pointProfile || embankmentConfig.get().pointProfile->points.empty())
    {
        embankmentConfig = m_config->getSurfaceData(
            +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);
    }

    GenerateMeshFromSplineArgs args;
    args.dstMesh = sharedData.mesh;
    args.adaptiveDividing = true;

    for (int pathIdx = 0; pathIdx < m_splines.size(); ++pathIdx)
    {
        args.splineList = m_splines[pathIdx].buildSegmentsVector();
        if (args.splineList.empty())
        {
            continue;
        }

        args.surfaceName = "EmbankmentPath" + pathIdx;
        args.profileOffset.z = 0.005 * pathIdx;
        if (embankmentConfig.get().pointProfile &&
            !embankmentConfig.get().pointProfile->points.empty())
        {
            generateSurface(sharedData, embankmentConfig.get(), args);
            generateCaps(String::format("EmbankmentCap%d", pathIdx), embankmentConfig.get(),
                m_neighborConfigs[pathIdx], m_splines[pathIdx]);

            m_temporaryMaterialsInfo.emplace_back(
                OutSurfaceData{args.surfaceName, embankmentConfig.get()});
        }

        const size_t surfacesCount = m_config->getSurfacesData().size();
        if ((surfacesCount > +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::Total) &&
            isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::JunctionAdditional))
        {
            for (int surfaceIndex = +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::Total;
                    surfaceIndex < surfacesCount; ++surfaceIndex)
            {
                auto& config = m_config->getSurfaceData(surfaceIndex);
                if (!config.pointProfile || config.pointProfile->points.empty())
                {
                    continue;
                }

                args.surfaceName = "EmbankmentPath" + pathIdx + config.name;

                if (config.profileOffset.length2() > MinimalSplineProfileOffset2)
                {
                    std::vector<BezierSegment> tmpSplineList(std::move(args.splineList));

                    auto offsetedSpline =
                        m_splines[pathIdx].createOffseted(toGlm(config.profileOffset));
                    args.splineList = offsetedSpline.buildSegmentsVector();
                    generateSurface(sharedData, config, args);
                    generateCaps(String::format("Additional%dCap%d", surfaceIndex, pathIdx), config,
                        m_neighborConfigs[pathIdx], offsetedSpline);

                    args.splineList = std::move(tmpSplineList);
                }
                else
                {
                    generateSurface(sharedData, config, args);
                    generateCaps(String::format("Additional%dCap%d", surfaceIndex, pathIdx), config,
                        m_neighborConfigs[pathIdx], m_splines[pathIdx]);
                }

                if (config.defaultMaterialInfo.hasParamsToApply())
                {
                    m_temporaryMaterialsInfo.emplace_back(OutSurfaceData{args.surfaceName, config});
                }
            }
        }
    }
}

void
DynamicRailwayJunctionBase::addRail(SharedDataArgsBase& sharedData, int srcPathIdx, int dstRailIdx,
    const char* name, double pathOffset, bool mirror)
{
    sharedData.railInfoById[dstRailIdx] = {name, mirror};
    sharedData.splineById[dstRailIdx] = getSpline(sharedData, srcPathIdx)
        .createOffseted(glm::dvec3(0., pathOffset, 0.));
    sharedData.splineSearchById[dstRailIdx].build(sharedData.splineById[dstRailIdx]);
    sharedData.sourcePathSplineIds[dstRailIdx] = srcPathIdx;
}

void DynamicRailwayJunctionBase::generateRail(SharedDataArgsBase& sharedData, int splineIdx,
    const char* surfaceName, bool profileMirrorY, double trimStart, std::optional<double> trimEnd)
{
    BezierSpline const& currentSpline = sharedData.splineById[splineIdx];

    struct TrimBound
    {
        double distance;
        std::optional<Unigine::Math::Vec3> connectionPoint;
    };
    std::vector<TrimBound> trimBounds;
    trimBounds.reserve(6);
    trimBounds.push_back({ trimStart });

    for (const auto& railFrogData : sharedData.railFrogsInfo)
    {
        if (railFrogData->leftInputSplineIdx != splineIdx
            && railFrogData->rightInputSplineIdx != splineIdx)
        {
            continue;
        }

        auto const& splineInterval = (railFrogData->leftInputSplineIdx == splineIdx)
            ? railFrogData->leftInputSplineInterval
            : railFrogData->rightInputSplineInterval;
        if (!splineInterval)
        {
            continue;
        }

        double startBound = std::min(splineInterval->first, splineInterval->second);
        double endBound = std::max(splineInterval->first, splineInterval->second);
        if (endBound <= trimStart)
        {
            continue;
        }

        auto connectionIt = railFrogData->connections.find(splineIdx);
        if (connectionIt != railFrogData->connections.end())
        {
            trimBounds.push_back({ startBound, connectionIt->second[0] });
            trimBounds.push_back({ endBound, connectionIt->second[1] });
        }
        else
        {
            trimBounds.push_back({ startBound });
            trimBounds.push_back({ endBound });
        }
    }

    if (trimEnd)
    {
        trimBounds.push_back({*trimEnd});
    }
    else
    {
        trimBounds.push_back({currentSpline.arcLength()});
    }

    std::sort(trimBounds.begin(), trimBounds.end(), [](const TrimBound& a, const TrimBound& b)
        {
            return a.distance < b.distance;
        }
    );

    GenerateMeshFromSplineArgs args;
    args.dstMesh = sharedData.mesh;

    assert(trimBounds.size() % 2 == 0);
    int partIdx = 0;
    for (std::size_t i = 0; i < trimBounds.size(); i += 2)
    {
        const auto& startBound = trimBounds[i];
        const auto& endBound = trimBounds[i + 1];

        if (std::abs(startBound.distance - endBound.distance) <
            std::numeric_limits<double>::epsilon())
        {
            continue;
        }

        args.postCap = args.preCap = false;
        if (i == 0)
        {
            auto pathIdx = getSrcPathIdx(sharedData, splineIdx);
            if (pathIdx &&
                (!m_neighborConfigs[*pathIdx].previous ||
                    m_neighborConfigs[*pathIdx].previous->empty()))
            {
                args.preCap = true;
            }
        }
        if (i + 2 == trimBounds.size())
        {
            auto pathIdx = getSrcPathIdx(sharedData, splineIdx);
            if (pathIdx &&
                (!m_neighborConfigs[*pathIdx].next || m_neighborConfigs[*pathIdx].next->empty()))
            {
                args.postCap = true;
            }
        }

        BezierSpline trimmedSpline = currentSpline.createTrimmed(startBound.distance,
            endBound.distance);

        args.splineList = trimmedSpline.buildSegmentsVector();
        if (args.splineList.empty())
        {
            warning("splineById[%d].createTrimmed(%f, %f).buildSegmentsVector() empty\n", splineIdx,
                startBound.distance, endBound.distance);
            continue;
        }
        if (startBound.connectionPoint)
        {
            auto& modifySegment = args.splineList.front();
            modifySegment = BezierSegment(toGlm(*startBound.connectionPoint), modifySegment.p1(),
                modifySegment.p2(), modifySegment.p3());
        }
        if (endBound.connectionPoint)
        {
            auto& modifySegment = args.splineList.back();
            modifySegment = BezierSegment(modifySegment.p0(), modifySegment.p1(),
                modifySegment.p2(), toGlm(*endBound.connectionPoint));
        }
        args.surfaceName = surfaceName;
        if (trimBounds.size() > 2 && (partIdx > 0))
        {
            args.surfaceName += "_" + std::to_string(partIdx);
        }
        args.profileMirrorY = profileMirrorY;
        generateRailSurfaces(sharedData, args);

        partIdx++;
    }
}

void
DynamicRailwayJunctionBase::generateRails(SharedDataArgsBase& sharedData)
{
    for (const auto& [splineIdx, railData] : sharedData.railInfoById)
    {
        double trimStart = 0.;
        std::optional<double> trimEnd;

        auto pointRailsIt = sharedData.pointRailsBySplineId.find(splineIdx);
        if (pointRailsIt != sharedData.pointRailsBySplineId.end())
        {
            if (auto frontArrow = pointRailsIt->second.front())
            {
                trimStart =
                    std::max(frontArrow->end.distanceOffset, frontArrow->start.distanceOffset);
            }
            if (auto backArrow = pointRailsIt->second.back())
            {
                trimEnd = std::min(backArrow->start.distanceOffset, backArrow->end.distanceOffset);
            }
        }

        generateRail(sharedData, splineIdx, railData.surfaceName.data(), railData.mirrorY,
            trimStart, trimEnd);
    } // for (const auto& [splineIdx, railData] : sharedData.railsDataTable)
}

// RailFrogs functions

DynamicRailwayJunctionBase::RailFrogInfoPtr
DynamicRailwayJunctionBase::findFirstRailFrog(SharedDataArgsBase& sharedData, int splineIdx,
    double fromDistance, double toDistance) const
{
    double minDistance = std::min(fromDistance, toDistance);
    double maxDistance = std::max(fromDistance, toDistance);

    DynamicRailwayJunctionBase::RailFrogInfoPtr result;
    for (auto& railFrogInfo : sharedData.railFrogsInfo)
    {
        if (railFrogInfo->leftInputSplineIdx == splineIdx)
        {
            const auto& interval = railFrogInfo->leftInputSplineInterval;
            if (interval)
            {
                if (minDistance <= interval->second && maxDistance >= interval->first)
                {
                    if (!result ||
                        (std::abs(interval->first - fromDistance) <
                            std::abs(interval->first - fromDistance)))
                    {
                        result = railFrogInfo;
                    }
                }
            }
            else
            {
                const double distance = railFrogInfo->crossPoint.first.distanceOffset;
                if (minDistance <= distance && maxDistance >= distance)
                {
                    if (!result ||
                        (std::abs(distance - fromDistance) < std::abs(distance - fromDistance)))
                    {
                        result = railFrogInfo;
                    }
                }
            }
        }
        else if (railFrogInfo->rightInputSplineIdx == splineIdx)
        {
            const auto& interval = railFrogInfo->rightInputSplineInterval;
            if (interval)
            {
                if (minDistance <= interval->second && maxDistance >= interval->first)
                {
                    if (!result ||
                        (std::abs(interval->first - fromDistance) <
                            std::abs(interval->first - fromDistance)))
                    {
                        result = railFrogInfo;
                    }
                }
            }
            else
            {
                const double distance = railFrogInfo->crossPoint.second.distanceOffset;
                if (minDistance <= distance && maxDistance >= distance)
                {
                    if (!result ||
                        (std::abs(distance - fromDistance) < std::abs(distance - fromDistance)))
                    {
                        result = railFrogInfo;
                    }
                }
            }
        }
    }
    return result;
}

DynamicRailwayJunctionBase::RailFrogInfoPtr
DynamicRailwayJunctionBase::findRailFrogBySplines(SharedDataArgsBase& sharedData, int splineIdx1,
    int splineIdx2) const
{
    for (auto& railFrogInfo : sharedData.railFrogsInfo)
    {
        if ((railFrogInfo->leftInputSplineIdx == splineIdx1) &&
            (railFrogInfo->rightInputSplineIdx == splineIdx2))
        {
            return railFrogInfo;
        }
        else if ((railFrogInfo->leftInputSplineIdx == splineIdx2) &&
            (railFrogInfo->rightInputSplineIdx == splineIdx1))
        {
            return railFrogInfo;
        }
        
    }
    return nullptr;
}

DynamicRailwayJunctionBase::RailFrogInfoPtr
DynamicRailwayJunctionBase::getRailFrogAt(SharedDataArgsBase& sharedData, int splineIdx,
    double distance, bool forMounts) const
{
    for (auto& railFrogInfo : sharedData.railFrogsInfo)
    {
        if (railFrogInfo->leftInputSplineIdx == splineIdx)
        {
            const auto& interval = forMounts ? railFrogInfo->leftInputSplineMountsInterval
                                             : railFrogInfo->leftInputSplineInterval;
            if (interval && distance >= interval->first && distance <= interval->second)
            {
                return railFrogInfo;
            }
        }
        else if (railFrogInfo->rightInputSplineIdx == splineIdx)
        {
            const auto& interval = forMounts ? railFrogInfo->rightInputSplineMountsInterval
                                             : railFrogInfo->rightInputSplineInterval;
            if (interval && distance >= interval->first && distance <= interval->second)
            {
                return railFrogInfo;
            }
        }
    }
    return nullptr;
}

DynamicRailwayJunctionBase::RailFrogInfoPtr
DynamicRailwayJunctionBase::generateRailFrog(SharedDataArgsBase& sharedData, int leftInputSplineIdx,
    int rightInputSplineIdx, bool flip, bool isObtuse, bool validateLeftRight)
{
    BezierSpline& spline1 = sharedData.splineById[leftInputSplineIdx];
    BezierSpline& spline2 = sharedData.splineById[rightInputSplineIdx];
    TransformSpline& splineSearch1 = sharedData.splineSearchById[leftInputSplineIdx];
    TransformSpline& splineSearch2 = sharedData.splineSearchById[rightInputSplineIdx];

    const std::optional<TransformSpline::NeighbourPoints> pointsOnSpline =
        splineSearch1.findClosestPoints(splineSearch2);
    if (!pointsOnSpline || (*pointsOnSpline).gap > 0.1)
    {
        error("Can not find intersection between rail splines %d and %d\n", leftInputSplineIdx,
            rightInputSplineIdx);
        return nullptr;
    }

    const TransformSpline::Point crossPoint1 = (*pointsOnSpline).first;
    const TransformSpline::Point crossPoint2 = (*pointsOnSpline).second;
    const auto tangent1 = crossPoint1.transform.getAxisX();
    const auto tangent2 = crossPoint2.transform.getAxisX();

    if (validateLeftRight && (Math::cross(tangent1, tangent2).z < 0.0))
    {
        warning("Swapped left/right splines detected, fix\n");
        return generateRailFrog(sharedData, rightInputSplineIdx, leftInputSplineIdx, flip,
            isObtuse);
    }

    auto railFrogData = std::make_shared<RailFrogInfo>();

    railFrogData->leftInputSplineIdx = leftInputSplineIdx;
    railFrogData->rightInputSplineIdx = rightInputSplineIdx;

    railFrogData->crossPoint = (*pointsOnSpline);
    auto railFrogPosition = crossPoint1.transform.getTranslate();
    auto railFrogDirection = Math::normalize(tangent1 + tangent2);
    railFrogData->transform = transformFromPositionAndTangent(railFrogPosition,
        flip  ? -railFrogDirection : railFrogDirection);
    railFrogData->angle = std::abs(wrapValueAround(normalizedAngleFromToDeg(tangent1, tangent2), -90, 90));
    railFrogData->isFlipped = flip;
    railFrogData->isObtuse = isObtuse;

    if (m_config->getRailFrogData().skinned)
    {
        chooseRailFrogNodeSkinned(sharedData, *railFrogData, isObtuse ? "_T" : "_A");
    }
    else
    {
        chooseRailFrogNodeStatic(sharedData, *railFrogData, isObtuse ? "_T" : "_A");
    }

    if (railFrogData->object && !m_config->getRailFrogData().materialInfo.name.empty())
    {
        m_setChildNodeMaterialInfo.emplace_back(SetMaterialChildNodeData{
            railFrogData->object->getName(), m_config->getRailFrogData().materialInfo});
    }

    sharedData.railFrogsInfo.push_back(railFrogData);

    auto pullSplineOnFrog = [this](BezierSpline& spline, TransformSpline& splineSearch,
                  std::pair<double, double>& interval, const std::array<Math::Vec3, 2>& connections)
    {
        std::vector<BezierSpline::PointToInsert> points;
        auto glmInConnection = toGlm(connections[0]);
        points.emplace_back(BezierSpline::PointToInsert{interval.first, glmInConnection});
        auto glmOutConnection = toGlm(connections[1]);
        points.emplace_back(BezierSpline::PointToInsert{interval.second, glmOutConnection});
        if (!spline.insertPoints(points))
        {
            return false;
        }

        splineSearch.build(spline);
        auto splineInPoint = splineSearch.findClosestPoint(glmInConnection);
        auto splineOutPoint = splineSearch.findClosestPoint(glmOutConnection);
        if (!splineInPoint || !splineOutPoint)
        {
            error("Can not update spline interval\n");
            return false;
        }
        interval = std::make_pair(splineInPoint->distanceOffset, splineOutPoint->distanceOffset);
        return true;
    };

    auto& leftInterval = railFrogData->leftInputSplineInterval;
    auto leftConnectionsIt = railFrogData->connections.find(railFrogData->leftInputSplineIdx);
    if (leftInterval && (leftConnectionsIt != railFrogData->connections.end()))
    {
        pullSplineOnFrog(spline1, splineSearch1, *leftInterval, leftConnectionsIt->second);
    }

    auto& rightInterval = railFrogData->rightInputSplineInterval;
    auto rightConnectionsIt = railFrogData->connections.find(railFrogData->rightInputSplineIdx);
    if (rightInterval && (rightConnectionsIt != railFrogData->connections.end()))
    {
        pullSplineOnFrog(spline2, splineSearch2, *rightInterval, rightConnectionsIt->second);
    }

    return railFrogData;
}

bool
DynamicRailwayJunctionBase::chooseRailFrogNodeSkinned(SharedDataArgsBase& sharedData,
    RailFrogInfo& railFrogData, const char* suffix)
{
    message("generating rail frog...\n");

    auto knotModel = selectKnotFileSkinned(railFrogData.angle, suffix);
    if (!knotModel)
    {
        return false;
    }

    auto railFrogNode = MetaUnigine::loadNode(knotModel->filePath.data());
    if (!railFrogNode)
    {
        error("RailwayYJunctionMeshGenerator::generateRailFrog\n");
        error("Can not load skinned mesh: %s\n", knotModel->filePath.data());
        return false;
    }
    auto skinnedObject = std::dynamic_pointer_cast<MetaUnigine::ObjectMeshSkinned>(railFrogNode);

    int animIdx = -1;
    for (int layerIdx = 0; layerIdx < skinnedObject->getNumLayers(); ++layerIdx)
    {
        if (!skinnedObject->isLayerEnabled(layerIdx))
        {
            continue;
        }
        animIdx = skinnedObject->getAnimation(layerIdx);
        if (animIdx >= 0)
        {
            break;
        }
    }
    if (animIdx >= 0)
    {
        // Apply dynamic angle
        float animOffset = (railFrogData.angle - knotModel->railFrogAngleDegreesMin) /
            (knotModel->railFrogAngleDegreesMax - knotModel->railFrogAngleDegreesMin);
        float time = (skinnedObject->getNumAnimationFrames(animIdx) - 1) * animOffset;
        skinnedObject->setTime(time);
        skinnedObject->setSpeed(0.0f);
        skinnedObject->play();
    }

    // Calculate connection points and spline intervals
    struct FrogIntervalsAndConnections
    {
        std::array<Unigine::Math::Vec3, 2> connections;
        std::optional<std::pair<double, double>> splineInterval;
        std::optional<std::pair<double, double>> mountsInterval;
    };
    auto calcFrogIntervalsAndConnections =
        [&skinnedObject, &railFrogData](const char* inBoneName, const char* outBoneName,
            double crossPointDistance,
            std::optional<double> railFrogEndDistance) -> std::optional<FrogIntervalsAndConnections>
    {
        int inBoneIdx = skinnedObject->findBone(inBoneName);
        int outBoneIdx = skinnedObject->findBone(outBoneName);
        if (inBoneIdx >= 0 && outBoneIdx >= 0)
        {
            FrogIntervalsAndConnections result;

            if (railFrogData.isFlipped)
            {
                std::swap(inBoneIdx, outBoneIdx);
            }

            Math::Vec3 inTransform(skinnedObject->getBoneTransform(inBoneIdx).getTranslate());
            Math::Vec3 outTransform(skinnedObject->getBoneTransform(outBoneIdx).getTranslate());

            result.connections[0] = railFrogData.transform * inTransform;
            result.connections[1] = railFrogData.transform * outTransform;

            double begin = crossPointDistance - inTransform.length();
            double end = crossPointDistance + outTransform.length();

            result.splineInterval = std::make_pair(begin, end);

            if (railFrogEndDistance)
            {
                if (!railFrogData.isFlipped)
                {
                    begin = *railFrogEndDistance;
                }
                else
                {
                    end = *railFrogEndDistance;
                }
            }
            result.mountsInterval = std::make_pair(begin, end);

            return result;
        }
        return std::nullopt;
    };

    std::optional<double> railFrogFirstSplineEndDistance;
    std::optional<double> railFrogSecondSplineEndDistance;

    double offsetFromCenter = -0.2; // Add offset for hollow edge - mounts will bypass it
    if (auto railFrogEnd = getSkinnedRailFrogClosurePoint(sharedData, railFrogData,
            SkinnedFrogGrailEndBone, offsetFromCenter))
    {
        railFrogFirstSplineEndDistance = railFrogEnd->first.distanceOffset;
        railFrogSecondSplineEndDistance = railFrogEnd->second.distanceOffset;
    }

    if (auto intervalsAndConnections =
            calcFrogIntervalsAndConnections(SkinnedFrogRail1InBone, SkinnedFrogRail1OutBone,
                railFrogData.crossPoint.first.distanceOffset, railFrogFirstSplineEndDistance))
    {
        if (intervalsAndConnections->splineInterval &&
            findFirstRailFrog(sharedData, railFrogData.leftInputSplineIdx,
                intervalsAndConnections->splineInterval->first,
                intervalsAndConnections->splineInterval->second))
        {
            return false;
        }

        railFrogData.connections[railFrogData.leftInputSplineIdx] =
            intervalsAndConnections->connections;
        railFrogData.leftInputSplineInterval = intervalsAndConnections->splineInterval;
        railFrogData.leftInputSplineMountsInterval = intervalsAndConnections->mountsInterval;
    }
    if (auto intervalsAndConnections =
            calcFrogIntervalsAndConnections(SkinnedFrogRail2InBone, SkinnedFrogRail2OutBone,
                railFrogData.crossPoint.second.distanceOffset, railFrogSecondSplineEndDistance))
    {
        if (intervalsAndConnections->splineInterval &&
            findFirstRailFrog(sharedData, railFrogData.rightInputSplineIdx,
                intervalsAndConnections->splineInterval->first,
                intervalsAndConnections->splineInterval->second))
        {
            railFrogData.connections.clear();
            railFrogData.leftInputSplineInterval = std::nullopt;
            railFrogData.leftInputSplineMountsInterval = std::nullopt;

            return false;
        }

        railFrogData.connections[railFrogData.rightInputSplineIdx] =
            intervalsAndConnections->connections;
        railFrogData.rightInputSplineInterval = intervalsAndConnections->splineInterval;
        railFrogData.rightInputSplineMountsInterval = intervalsAndConnections->mountsInterval;
    }

    if (railFrogData.connections.size() == 2)
    {
        auto firstIt = railFrogData.connections.begin();
        auto checkTangent1 = firstIt->second[0] - firstIt->second[1];
        checkTangent1.normalize();

        auto secondIt = ++railFrogData.connections.begin();
        auto checkTangent2 = secondIt->second[0] - secondIt->second[1];
        checkTangent2.normalize();

        double checkAngle =
            std::abs(wrapValueAround(normalizedAngleFromToDeg(checkTangent1, checkTangent2), -90, 90));
        if (std::abs(railFrogData.angle - checkAngle) > 0.5)
        {
            Log::warning("Junction %d frog \"%s\" angles mismatch, required %f -> result %f\n",
                getKey().id, knotModel->filePath.data(), railFrogData.angle, checkAngle);
        }
    }

    railFrogNode->setTransform(railFrogData.transform);
    railFrogNode->setName(makeChildNameUniqueIndexed(m_object, String("RailFrogNode")));
    m_object->addChild(railFrogNode);
    railFrogData.object = skinnedObject;

    return true;
}

std::optional<DynamicRailwayJunctionBase::SelectedKnotSkinnedFile>
DynamicRailwayJunctionBase::selectKnotFileSkinned(double railFrogAngleDegrees, const char* suffix)
{
    std::optional<SelectedKnotSkinnedFile> result;

    std::string dataPath = Engine::get()->getDataPath();
    std::string knotModelsDirectory = dataPath + m_config->getRailFrogData().directory;
    DirPtr dir = Dir::create();
    if (dir->open(knotModelsDirectory.c_str()))
    {
        int numFiles = dir->getNumFiles();
        for (int i = 0; i < numFiles; i++)
        {
            std::string fileName = dir->getFileName(i);

            // Example of matched name : cross_L2862_(6_9)_A.node, where
            // '6' means 'fromAngle', '9' means 'toAngle', '_A' means sharp frog.
            std::string patternStr("\\(([\\d.]*_[\\d.]*)\\)");
            std::regex pattern(patternStr + suffix + ".node");
            std::smatch matches;
            if (std::regex_search(fileName, matches, pattern))
            {
                auto toAngleIt = std::find(matches[1].first, matches[1].second, '_');
                if (toAngleIt == matches[1].second)
                {
                    continue;
                }

                double fromAngle = std::stod(matches[1]);
                double toAngle = std::stod(&*(toAngleIt + 1));
                if ((fromAngle <= railFrogAngleDegrees) && (railFrogAngleDegrees <= toAngle))
                {
                    result = SelectedKnotSkinnedFile{fileName.substr(fileName.find("data/") + 5),
                        fromAngle, toAngle};
                    break;
                }
            }
        } // for (int i = 0; i < numFiles; i++)
        dir->close();
    }
    else
    {
        error("Can not open directory: %s\n", knotModelsDirectory.c_str());
    }

    return result;
}

bool DynamicRailwayJunctionBase::chooseRailFrogNodeStatic(SharedDataArgsBase& sharedData,
    RailFrogInfo& railFrogData, const char* suffix)
{
    message("generating rail frog...\n");
    
    auto knotModel = selectKnotFileStatic(railFrogData.angle, suffix);
    if (!knotModel)
    {
        return false;
    }

    auto knotModelLength = getKnotModelLength(knotModel->filePath);
    if (!knotModelLength)
    {
        return false;
    }

    railFrogData.angle = knotModel->railFrogAngleDegrees;
    double halfRailFrogLength = *knotModelLength * 0.5;
    float halfRailFrogAngle = railFrogData.angle * 0.5f;

    double leftClosureSplineLength = sharedData.splineById[railFrogData.leftInputSplineIdx].arcLength();
    double railFrogLeftEndLength =
        railFrogData.crossPoint.first.distanceOffset + halfRailFrogLength;
    if (leftClosureSplineLength < railFrogLeftEndLength)
    {
        double lackOfSpace = railFrogLeftEndLength - leftClosureSplineLength;
        warning("Rail frog goes beyound end of left path of the junction\n");
        warning("We recommend extending the left paths by %.2f meters\n", lackOfSpace);
    }

    double rightClosureSplineLength = sharedData.splineById[railFrogData.rightInputSplineIdx].arcLength();
    double railFrogRightEndLength =
        railFrogData.crossPoint.second.distanceOffset + halfRailFrogLength;
    if (rightClosureSplineLength < railFrogRightEndLength)
    {
        double lackOfSpace = railFrogRightEndLength - rightClosureSplineLength;
        warning("Rail frog goes beyound end of right path of the junction\n");
        warning("We recommend extending the right paths by %.2f meters\n", lackOfSpace);
    }

    // Calculate spline intervals

    double offset = halfRailFrogLength / Math::cos(halfRailFrogAngle * Math::Consts::DEG2RAD_D);
    double leftBegin = railFrogData.crossPoint.first.distanceOffset - offset;
    double leftEnd = railFrogData.crossPoint.first.distanceOffset + offset;
    double rightBegin = railFrogData.crossPoint.second.distanceOffset - offset;
    double rightEnd = railFrogData.crossPoint.second.distanceOffset + offset;

    if (findFirstRailFrog(sharedData, railFrogData.leftInputSplineIdx, leftBegin, leftEnd))
    {
        return false;
    }

    if (findFirstRailFrog(sharedData, railFrogData.rightInputSplineIdx, rightBegin, rightEnd))
    {
        return false;
    }

    auto railFrogNode = MetaUnigine::loadNode(knotModel->filePath.c_str());
    if (!railFrogNode)
    {
        error("RailwayYJunctionMeshGenerator::generateRailFrog\n");
        error("Can not load static mesh: %s\n", knotModel->filePath.c_str());
        return false;
    }

    railFrogNode->setTransform(railFrogData.transform);
    railFrogNode->setName(makeChildNameUniqueIndexed(m_object, String("RailFrogNode")));
    m_object->addChild(railFrogNode);
    railFrogData.object
        = std::dynamic_pointer_cast<MetaUnigine::ObjectMeshStatic>(railFrogNode);

    // Calculate connection points

    const auto railFrogPosition = railFrogData.transform.getTranslate();
    const auto tangent1 = railFrogData.crossPoint.first.transform.getAxisX();
    const auto tangent2 = railFrogData.crossPoint.second.transform.getAxisX();
    const auto railFrogDirection = Math::normalize(tangent1 + tangent2);

    auto endPoint = railFrogDirection * halfRailFrogLength;

    double sign1 = Math::sign(Math::cross(railFrogDirection, tangent1).z);
    railFrogData.connections[railFrogData.leftInputSplineIdx][0] =
        railFrogPosition + Math::quat(0.0f, 0.0f, halfRailFrogAngle * sign1) * -endPoint;
    railFrogData.connections[railFrogData.leftInputSplineIdx][1] =
        railFrogPosition + Math::quat(0.0f, 0.0f, halfRailFrogAngle * sign1) * endPoint;

    double sign2 = Math::sign(Math::cross(railFrogDirection, tangent2).z);
    railFrogData.connections[railFrogData.rightInputSplineIdx][0] =
        railFrogPosition + Math::quat(0.0f, 0.0f, halfRailFrogAngle * sign2) * -endPoint;
    railFrogData.connections[railFrogData.rightInputSplineIdx][1] =
        railFrogPosition + Math::quat(0.0f, 0.0f, halfRailFrogAngle * sign2) * endPoint;

    railFrogData.leftInputSplineInterval = std::make_pair(leftBegin, leftEnd);
    railFrogData.rightInputSplineInterval = std::make_pair(rightBegin, rightEnd);

    double offsetFromCenter = -0.2;
    auto railFrogEnd = getStaticRailFrogClosurePoint(sharedData, railFrogData, FrogGrailPointEnd,
        offsetFromCenter); // Add offset for hollow edge - mounts will bypass it
    if (railFrogEnd)
    {
        if (!railFrogData.isFlipped)
        {
            leftBegin = railFrogEnd->first.distanceOffset;
            rightBegin = railFrogEnd->second.distanceOffset;
        }
        else
        {
            leftEnd = railFrogEnd->first.distanceOffset;
            rightEnd = railFrogEnd->second.distanceOffset;
        }
    }

    railFrogData.leftInputSplineMountsInterval = std::make_pair(leftBegin, leftEnd);
    railFrogData.rightInputSplineMountsInterval = std::make_pair(rightBegin, rightEnd);

    return true;
}

std::optional<DynamicRailwayJunctionBase::SelectedKnotFile>
DynamicRailwayJunctionBase::selectKnotFileStatic(double railFrogAngleDegrees, const char* suffix)
{
    double degreesFloor = Math::floor(railFrogAngleDegrees);
    double degreesCeil = Math::ceil(railFrogAngleDegrees);
    if (Math::abs(railFrogAngleDegrees - degreesFloor) < Math::abs(railFrogAngleDegrees - degreesCeil))
    {
        railFrogAngleDegrees = degreesFloor;
    }
    else
    {
        railFrogAngleDegrees = degreesCeil;
    }

    std::string knotModelPath;
    std::string angleKey;

    {
        std::stringstream ss;
        ss << suffix << (int)railFrogAngleDegrees << "_";
        angleKey = ss.str();
    }

    std::string dataPath = Engine::get()->getDataPath();
    std::string knotModelsDirectory = dataPath + m_config->getRailFrogData().directory;
    DirPtr dir = Dir::create();
    if (dir->open(knotModelsDirectory.c_str()))
    {
        int numFiles = dir->getNumFiles();
        for (int i = 0; i < numFiles; i++)
        {
            std::string fileName = dir->getFileName(i);
            std::string extension = fileName.substr(fileName.find_last_of('.'));
            if (extension != ".node")
            {
                continue;
            }

            if (fileName.find(angleKey) != std::string::npos)
            {
                knotModelPath = fileName.substr(fileName.find("data/") + 5);

                break;
            }
        }//for (int i = 0; i < numFiles; i++)

        dir->close();
    }
    else
    {
        error("Can not open directory: %s\n", knotModelsDirectory.c_str());
        return std::nullopt;
    }
    return SelectedKnotFile{ knotModelPath, railFrogAngleDegrees };
}

std::optional<double>
DynamicRailwayJunctionBase::getKnotModelLength(const std::string& knotModelPath)
{
    std::string fileName = knotModelPath.substr(knotModelPath.find_last_of('/') + 1);
    size_t lengthSuffixPosBegin = fileName.find("_L");
    if (lengthSuffixPosBegin == std::string::npos)
    {
        error("Can not determine length of rail frog model: %s\n", fileName.c_str());
        return std::nullopt;
    }

    size_t lengthSuffixPosEnd = std::string::npos;
    for (lengthSuffixPosEnd = lengthSuffixPosBegin + 2;
        lengthSuffixPosEnd < fileName.size(); lengthSuffixPosEnd++)
    {
        if (!isdigit(fileName[lengthSuffixPosEnd]))
        {
            break;
        }
    }

    lengthSuffixPosBegin += 2; // '_L' - 2 characters
    size_t suffixLength = lengthSuffixPosEnd - lengthSuffixPosBegin;
    if (suffixLength <= 0)
    {
        error("Can not determine length of rail frog model: %s\n", fileName.c_str());
        return std::nullopt;
    }

    std::string lengthSuffix = fileName.substr(lengthSuffixPosBegin, suffixLength);
    int lengthMm = atoi(lengthSuffix.c_str());
    return RailwayMeshGeneratorConfig::toInnerUnits(lengthMm);
}

// GuardRails

std::optional<DynamicRailwayJunctionBase::GuardRailInfoRef>
DynamicRailwayJunctionBase::getGuardAt(SharedDataArgsBase& sharedData, int splineIdx,
    double distance, std::optional<bool> onlyLeft) const
{
    auto getGuardAtLambda = [&splineIdx, &distance](
                                std::map<int, std::list<GuardRailInfo>>& guardRailsBySplineId)
        -> std::optional<DynamicRailwayJunctionBase::GuardRailInfoRef>
    {
        auto guardsIt = guardRailsBySplineId.find(splineIdx);
        if (guardsIt == guardRailsBySplineId.end())
        {
            return std::nullopt;
        }
        for (auto& guard : guardsIt->second)
        {
            if ((distance >= guard.start.distanceOffset) && (distance <= guard.end.distanceOffset))
            {
                return guard;
            }
        }
        return std::nullopt;
    };

    if (onlyLeft == std::nullopt || *onlyLeft)
    {
        return getGuardAtLambda(sharedData.leftGuardRailsBySplineId);
    }

    if (onlyLeft == std::nullopt || !*onlyLeft)
    {
        return getGuardAtLambda(sharedData.rightGuardRailsBySplineId);
    }

    return std::nullopt;
}

std::optional<DynamicRailwayJunctionBase::GuardRailInfoRef>
DynamicRailwayJunctionBase::getGuardAtInterval(SharedDataArgsBase& sharedData, int splineIdx,
    double fromDistance, double toDistance, std::optional<bool> onlyLeft) const
{
    auto getGuardAtLambda = [&splineIdx, &fromDistance, &toDistance](
                                std::map<int, std::list<GuardRailInfo>>& guardRailsBySplineId)
        -> std::optional<DynamicRailwayJunctionBase::GuardRailInfoRef>
    {
        auto guardsIt = guardRailsBySplineId.find(splineIdx);
        if (guardsIt == guardRailsBySplineId.end())
        {
            return std::nullopt;
        }
        for (auto& guard : guardsIt->second)
        {
            if ((fromDistance <= guard.end.distanceOffset) &&
                (toDistance >= guard.start.distanceOffset))
            {
                return guard;
            }
        }
        return std::nullopt;
    };

    if (onlyLeft == std::nullopt || *onlyLeft)
    {
        return getGuardAtLambda(sharedData.leftGuardRailsBySplineId);
    }

    if (onlyLeft == std::nullopt || !*onlyLeft)
    {
        return getGuardAtLambda(sharedData.rightGuardRailsBySplineId);
    }

    return std::nullopt;
}

void DynamicRailwayJunctionBase::generateGuardRailsForRailFrog(SharedDataArgsBase& sharedData,
    RailFrogInfo& railFrogInfo, double closureOffset)
{
    message("generating guard rails...\n");

    auto getLengthByName = [](std::string const& nodeName)-> double
    {
        int startPos = nodeName.find("_L");
        if (startPos == std::string::npos)
        {
            return 0;
        }

        startPos += 2;
        int endPos = startPos;
        for (; endPos < nodeName.size(); endPos++)
        {
            if (!isdigit(nodeName[endPos]))
            {
                break;
            }
        }

        std::string strLength = nodeName.substr(startPos, endPos - startPos);
        int lengthMm = atoi(strLength.c_str());

        return RailwayMeshGeneratorConfig::toInnerUnits(lengthMm);
    };

    const auto& splineSearch1 = sharedData.splineSearchById[railFrogInfo.leftInputSplineIdx];
    const auto& spline2 = sharedData.splineById[railFrogInfo.rightInputSplineIdx];
    const auto& crossPointDistance2 = railFrogInfo.crossPoint.second.distanceOffset;

    //seek for guard rail start
    auto guardRailBegin =
        getSkinnedRailFrogClosurePoint(sharedData, railFrogInfo, SkinnedFrogGrailBeginBone);
    if (!guardRailBegin)
    {
        guardRailBegin =
            getStaticRailFrogClosurePoint(sharedData, railFrogInfo, FrogGrailPointBegin);
    }
    if (!guardRailBegin)
    {
        const double startGap = RailwayMeshGeneratorConfig::toInnerUnits(
            m_config->getGuardRailsData().startGap);
        guardRailBegin = splineSearch1.findPointsWithDistance(spline2, startGap, 0.1,
            crossPointDistance2, 0.0);
        if (!guardRailBegin)
        {
            error("Can not find GuardRail begin\n");
            return;
        }
    }
    
    //seek for guard rail end
    auto guardRailEnd =
        getSkinnedRailFrogClosurePoint(sharedData, railFrogInfo, SkinnedFrogGrailEndBone);
    if (!guardRailEnd)
    {
        guardRailEnd = getStaticRailFrogClosurePoint(sharedData, railFrogInfo, FrogGrailPointEnd);
    }
    if (!guardRailEnd)
    {
        const double endGap = RailwayMeshGeneratorConfig::toInnerUnits(
            m_config->getGuardRailsData().endGap);
        guardRailEnd = splineSearch1.findPointsWithDistance(spline2, endGap, 0.1,
            crossPointDistance2);
        if (!guardRailEnd)
        {
            error("Can not calculate GuardRail end\n");
            return;
        }
    }

    if (railFrogInfo.isFlipped)
    {
        // This is because non-flipped frog takes path's left rail on left input,
        // flipped takes path's right rail on left input.
        // TODO: check compatibility with other usage scenarious, inverting can be done on calling -
        // as function argument
        closureOffset = -closureOffset;
    }

    std::string beginPartNodePath, endPartNodePath, middlePartNodePath;
    {
        std::stringstream ss;
        ss << m_config->getGuardRailsData().directory;
        ss << "/";
        ss << m_config->getGuardRailsData().leftNode;
        beginPartNodePath = ss.str();
    }

    {
        std::stringstream ss;
        ss << m_config->getGuardRailsData().directory;
        ss << "/";
        ss << m_config->getGuardRailsData().rightNode;
        endPartNodePath = ss.str();
    }

    {
        std::stringstream ss;
        ss << m_config->getGuardRailsData().directory;
        ss << "/";
        ss << m_config->getGuardRailsData().middleNode;
        middlePartNodePath = ss.str();
    }

    auto createGuardRailNodes = [this, &getLengthByName, & guardRailBegin, &guardRailEnd,
                                    &beginPartNodePath, &endPartNodePath, &middlePartNodePath,
                                    &sharedData]
    (int splineIdx, const char* rootName, const Math::Mat4& beginMat, const Math::Mat4& endMat,
        double closureOffset)
    {
        Math::Vec3 startPosition = beginMat * Math::Vec3(0., closureOffset, 0.);
        Math::Vec3 endPosition = endMat * Math::Vec3(0., closureOffset, 0.);

        Math::Vec3 direction = endPosition - startPosition;
        direction.normalize();

        Math::Vec3 leftToCenter = startPosition - beginMat.getTranslate();
        leftToCenter.normalize();

        bool isLeft = (Math::cross(direction, leftToCenter).z > 0.0);

        auto startPoint =
            getSplineSearch(sharedData, splineIdx).findClosestPoint(toGlm(startPosition));
        auto endPoint = getSplineSearch(sharedData, splineIdx).findClosestPoint(toGlm(endPosition));
        if (!startPoint || !endPoint)
        {
            return;
        }

        MetaUnigine::NodePtr beginNode, endNode, middleNode;
        auto guardRef = getGuardAtInterval(sharedData, splineIdx, startPoint->distanceOffset,
            endPoint->distanceOffset, isLeft);
        if (guardRef)
        {
            auto& guard = guardRef->get();

            if (guard.start.distanceOffset > startPoint->distanceOffset)
            {
                guard.start = *startPoint;
            }
            else
            {
                startPosition = guard.start.transform.getTranslate();
            }

            if (guard.end.distanceOffset < endPoint->distanceOffset)
            {
                guard.end = *endPoint;
            }
            else
            {
                endPosition = guard.end.transform.getTranslate();
            }

            beginNode = guard.node->findNode(GuardBeginNodeName);
            endNode = guard.node->findNode(GuardEndNodeName);
            middleNode = guard.node->findNode(GuardMiddleNodeName);
            if (!beginNode || !endNode || !middleNode)
            {
                return;
            }
        }
        else
        {
            auto guardRailRoot = MetaUnigine::NodeDummy::create();
            guardRailRoot->setName(rootName);
            guardRailRoot->setPosition(Math::Vec3_zero);
            m_object->addChild(guardRailRoot);

            beginNode = MetaUnigine::loadNode(beginPartNodePath.c_str());
            endNode = MetaUnigine::loadNode(endPartNodePath.c_str());
            middleNode = MetaUnigine::loadNode(middlePartNodePath.c_str());
            if (!beginNode || !endNode || !middleNode)
            {
                return;
            }

            beginNode->setName(GuardBeginNodeName);
            guardRailRoot->addChild(beginNode);
            endNode->setName(GuardEndNodeName);
            guardRailRoot->addChild(endNode);
            middleNode->setName(GuardMiddleNodeName);
            guardRailRoot->addChild(middleNode);

            auto& guardRailsBySplineId =
                isLeft ? sharedData.leftGuardRailsBySplineId : sharedData.rightGuardRailsBySplineId;
            guardRef = guardRailsBySplineId[splineIdx].emplace_back(
                GuardRailInfo{*startPoint, *endPoint, guardRailRoot});
        }

        if (!isLeft)
        {
            std::swap(endPosition, startPosition);
            direction = endPosition - startPosition;
            direction.normalize();
        }

        double length = (endPosition - startPosition).length();
        double beginPartLength = getLengthByName(m_config->getGuardRailsData().leftNode);
        double endPartLength = getLengthByName(m_config->getGuardRailsData().rightNode);
        double middlePartLength = getLengthByName(m_config->getGuardRailsData().middleNode);
        double scaleFactor = length / (beginPartLength + endPartLength + middlePartLength);
        double middlePartScaledLength = length - (beginPartLength + endPartLength);

        const Math::Mat4 rotation =
            transformFromPositionAndTangent(Math::Vec3_zero, direction);

        auto setNodeTransform = [](const MetaUnigine::NodePtr& node, const Math::Vec3& position,
                                    const Math::Mat4& rotation, double xScale, double offset)
        {
            Math::Mat4 matPosition, matOffset, transform, matScaling;
            matPosition.setTranslate(position);
            matOffset.setTranslate(Math::Vec3_right * offset * xScale);
            matScaling.setScale(Math::Vec3(xScale, 1.0, 1.0));

            transform = matPosition * rotation * matOffset * matScaling;

            node->setTransform(transform);
        };

        setNodeTransform(beginNode, startPosition, rotation, scaleFactor,
            beginPartLength);
        
        setNodeTransform(endNode, startPosition, rotation, scaleFactor,
            beginPartLength + endPartLength + middlePartLength);
        
        setNodeTransform(middleNode, startPosition, rotation, scaleFactor,
            beginPartLength + middlePartLength);
    };

    auto leftSplineId = getOppositeSplineId(sharedData, railFrogInfo.leftInputSplineIdx);
    if (leftSplineId)
    {
        createGuardRailNodes(*leftSplineId, "RightGuardRail",
            guardRailBegin->first.transform, guardRailEnd->first.transform, closureOffset);
    }
    auto rightSplineId = getOppositeSplineId(sharedData, railFrogInfo.rightInputSplineIdx);
    if (rightSplineId)
    {
        createGuardRailNodes(*rightSplineId, "LeftGuardRail",
            guardRailBegin->second.transform, guardRailEnd->second.transform, -closureOffset);
    }

    railFrogInfo.guardRails = true;
}

std::optional<TransformSpline::NeighbourPoints>
DynamicRailwayJunctionBase::getStaticRailFrogClosurePoint(SharedDataArgsBase& sharedData,
    const RailFrogInfo& railFrogInfo, const char* childName, std::optional<double> offsetFromCenter) const
{
    if (!railFrogInfo.object)
    {
        return std::nullopt;
    }
    int grailIdx = railFrogInfo.object->findChild(childName);
    if (grailIdx < 0)
    {
        return std::nullopt;
    }
    auto grailNode = railFrogInfo.object->getChild(grailIdx);
    if (!grailNode)
    {
        return std::nullopt;
    }

    auto grailPoint = grailNode->getPosition();
    if (offsetFromCenter)
    {
        grailPoint *= (1.0 + *offsetFromCenter / grailPoint.length());
    }

    auto frogPoint = toGlm(railFrogInfo.transform * grailPoint);
    auto point1 = sharedData.splineSearchById[railFrogInfo.leftInputSplineIdx].findClosestPoint(frogPoint);
    auto point2 = sharedData.splineSearchById[railFrogInfo.rightInputSplineIdx].findClosestPoint(frogPoint);
    if (!point1 || !point2)
    {
        return std::nullopt;
    }

    double distance = (point1->transform.getTranslate() - point2->transform.getTranslate()).
        length();
    return TransformSpline::NeighbourPoints{ distance, *point1, *point2 };
}

std::optional<TransformSpline::NeighbourPoints>
DynamicRailwayJunctionBase::getSkinnedRailFrogClosurePoint(SharedDataArgsBase& sharedData,
    const RailFrogInfo& railFrogInfo, const char* boneName,
    std::optional<double> offsetFromCenter) const
{
    if (!railFrogInfo.object)
    {
        return std::nullopt;
    }
    auto skinned = std::dynamic_pointer_cast<MetaUnigine::ObjectMeshSkinned>(railFrogInfo.object);
    if (!skinned)
    {
        return std::nullopt;
    }
    int grailIdx = skinned->findBone(boneName);
    if (grailIdx < 0)
    {
        return std::nullopt;
    }
    auto grailPoint = skinned->getBoneTransform(grailIdx).getTranslate();
    if (offsetFromCenter)
    {
        grailPoint *= (1.0 + *offsetFromCenter / grailPoint.length());
    }

    auto frogPoint = toGlm(railFrogInfo.transform * grailPoint);

    int splineId1 = railFrogInfo.leftInputSplineIdx;
    int splineId2 = railFrogInfo.rightInputSplineIdx;
    auto point1 = sharedData.splineSearchById[splineId1].findClosestPoint(frogPoint);
    auto point2 = sharedData.splineSearchById[splineId2].findClosestPoint(frogPoint);
    if (!point1 || !point2)
    {
        return std::nullopt;
    }

    double distance =
        (point1->transform.getTranslate() - point2->transform.getTranslate()).length();
    return TransformSpline::NeighbourPoints{distance, *point1, *point2};
}

// PointRails

std::optional<DynamicRailwayJunctionBase::HeelBlockInfoRef>
DynamicRailwayJunctionBase::getHeelBlockAt(SharedDataArgsBase& sharedData, int splineIdx,
    double distance) const
{
    auto heelsIt = sharedData.heelBlocksBySplineId.find(splineIdx);
    if (heelsIt == sharedData.heelBlocksBySplineId.end())
    {
        return std::nullopt;
    }
    for (auto& heel : heelsIt->second)
    {
        if ((distance >= heel.start) && (distance <= heel.end))
        {
            return heel;
        }
    }
    return std::nullopt;
}

void
DynamicRailwayJunctionBase::generateHeelBlock(SharedDataArgsBase& sharedData, int splineIdx,
    int otherSplineIdx, const TransformSpline::Point& end, bool leftSide)
{
    auto heelBlockRoot = MetaUnigine::NodeDummy::create();
    heelBlockRoot->setName(makeChildNameUniqueIndexed(getMetaObject(), String("HeelBlock")));
    getMetaObject()->addChild(heelBlockRoot);

    auto const& heelBlockData = m_config->getHeelBlockData();
    std::optional<double> scaleFactor;

    // outer part
    std::optional<Math::Mat4> outerTransform;
    const auto outerHeelPoint = sharedData.splineSearchById[otherSplineIdx].findClosestPoint(
        toGlm(end.transform.getTranslate()));

    sharedData.heelBlocksBySplineId[splineIdx].emplace_back(
        end.distanceOffset - heelBlockData.halfSize,
        end.distanceOffset + heelBlockData.halfSize
    );

    if (outerHeelPoint)
    {
        sharedData.heelBlocksBySplineId[otherSplineIdx].emplace_back(
            outerHeelPoint->distanceOffset - heelBlockData.halfSize,
            outerHeelPoint->distanceOffset + heelBlockData.halfSize
        );

        outerTransform = outerHeelPoint->transform;
        if (outerTransform)
        {
            std::string nodePath = heelBlockData.directory + "/" + heelBlockData.outerPart;
            if (auto node = MetaUnigine::loadNode(nodePath.c_str()))
            {
                node->setName(makeChildNameUniqueIndexed(heelBlockRoot, String("OuterPart")));
                heelBlockRoot->addChild(node);

                if (leftSide)
                {
                    node->setTransform(
                        *outerTransform * Math::Mat4(Math::quat(0.0f, 0.0f, 180.0f)));
                }
                else
                {
                    node->setTransform(*outerTransform);
                }

                if (!heelBlockData.materialInfo.name.empty())
                {
                    m_setChildNodeMaterialInfo.emplace_back(SetMaterialChildNodeData{
                        node->getName(), heelBlockData.materialInfo, heelBlockRoot->getName()});
                }
            }
        } // if (outerTransform)
    }     // if (outerHeelPoint)

    // inner part
    std::optional<Math::Mat4> innerTransform = end.transform;
    if (innerTransform)
    {
        std::string nodePath = heelBlockData.directory + "/" + heelBlockData.leftInnerPart;
        if (auto node = MetaUnigine::loadNode(nodePath.c_str()))
        {
            node->setName(makeChildNameUniqueIndexed(heelBlockRoot, String("InnerPart")));
            heelBlockRoot->addChild(node);

            if (leftSide)
            {
                node->setTransform(*innerTransform * Math::Mat4(Math::quat(0.0f, 0.0f, 180.0f)));
            }
            else
            {
                node->setTransform(*innerTransform);
            }

            if (!heelBlockData.materialInfo.name.empty())
            {
                m_setChildNodeMaterialInfo.emplace_back(SetMaterialChildNodeData{node->getName(),
                    heelBlockData.materialInfo, heelBlockRoot->getName()});
            }
        } // if (auto node = MetaUnigine::loadNode
    }     // if (innerTransform)

    // middle part
    if (outerTransform && innerTransform)
    {
        std::string nodePath = heelBlockData.directory + "/" + heelBlockData.middlePart;
        if (auto node = MetaUnigine::loadNode(nodePath.c_str()))
        {
            node->setName(makeChildNameUniqueIndexed(heelBlockRoot, String("MiddlePart")));
            heelBlockRoot->addChild(node);

            auto transform = Math::lerp(*outerTransform, *innerTransform, 0.5) *
                calcNodeScale(node, outerTransform->getTranslate(), innerTransform->getTranslate());

            node->setTransform(transform);

            if (!heelBlockData.materialInfo.name.empty())
            {
                m_setChildNodeMaterialInfo.emplace_back(SetMaterialChildNodeData{node->getName(),
                    heelBlockData.materialInfo, heelBlockRoot->getName()});
            }
        }
    }
}

void
DynamicRailwayJunctionBase::generateSwitchRods(SharedDataArgsBase& sharedData,
    const DynamicRailwayJunctionBase::PointRailInfo& leftPointRail,
    const DynamicRailwayJunctionBase::PointRailInfo& rightPointRail, const char* suffix,
    bool initialRightTransform,
    RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType)
{
    message("generating switch rods...\n");

    const Math::Mat4 leftTransforms[] = {leftPointRail.leftTransform,
        leftPointRail.rightTransform};
    const Math::Mat4 rightTransforms[] = {rightPointRail.leftTransform,
        rightPointRail.rightTransform};
    double pointRailsLength =
        std::abs(leftPointRail.end.distanceOffset - leftPointRail.start.distanceOffset);
    double pointRailSlicePoint = leftPointRail.slicePoint;

    auto calcPointRailTransform = [&leftTransforms, &rightTransforms, &leftPointRail,
                                      &rightPointRail, &pointRailsLength](bool leftToLeft,
                                      bool rightToLeft, double offsetFromRoot,
                                      const MetaUnigine::NodePtr& scaleNode = {})
    {
        const auto& leftTrans = leftTransforms[leftToLeft ? 0 : 1];
        const auto& rightTrans = rightTransforms[rightToLeft ? 0 : 1];

        Math::Mat4 result;
        Math::Vec3 leftPosition;
        Math::Vec3 rightPosition;

        double t = 1.0 - offsetFromRoot / pointRailsLength;
        auto leftSplineTransform = leftPointRail.joinedSpline
            ? leftPointRail.joinedSpline->calcSplineMatrixAtDistance(
                  t * leftPointRail.joinedSpline->arcLength())
            : std::nullopt;
        auto rightSplineTransform = rightPointRail.joinedSpline
            ? rightPointRail.joinedSpline->calcSplineMatrixAtDistance(
                  t * rightPointRail.joinedSpline->arcLength())
            : std::nullopt;
        if (leftSplineTransform && rightSplineTransform)
        {
            if (leftPointRail.isLeft)
            {
                if (!leftToLeft)
                {
                    leftSplineTransform = leftPointRail.rightTransform *
                        Math::inverse(leftPointRail.leftTransform) * *leftSplineTransform;
                }
            }

            if (!rightPointRail.isLeft)
            {
                if (rightToLeft)
                {
                    rightSplineTransform = rightPointRail.leftTransform *
                        Math::inverse(rightPointRail.rightTransform) * *rightSplineTransform;
                }
            }

            leftPosition = leftSplineTransform->getTranslate();
            rightPosition = rightSplineTransform->getTranslate();

            result = Math::lerp(*leftSplineTransform, *rightSplineTransform, 0.5);
        }
        else
        {
            const Math::Vec3 leftOrigin = leftTrans.getTranslate();
            const Math::Vec3 leftDirection(leftTrans.getAxisX());
            const Math::Vec3 rightOrigin = rightTrans.getTranslate();
            const Math::Vec3 rightDirection(rightTrans.getAxisX());

            Math::Vec3 dir(-(leftDirection + rightDirection).normalize());

            leftPosition = leftOrigin + (leftDirection * offsetFromRoot);
            rightPosition = rightOrigin + (rightDirection * offsetFromRoot);

            result = transformFromPositionAndTangent((leftPosition + rightPosition) * 0.5, dir);
        }
        if (scaleNode)
        {
            const Math::Mat4 scaleTransform = calcNodeScale(scaleNode, leftPosition, rightPosition);
            return result * scaleTransform;
        }
        return result;
    };

    auto addRodNode = [this, &sharedData, &calcPointRailTransform, &initialRightTransform, switchRodConfigType]
        (int rodIndex, double offsetFromRoot, const char* rodName, bool scale = true)
    {
        const auto& config = m_config->getSwitchRodsData(+switchRodConfigType);

        std::string sourceNodePath = config.rodNodePath;
        if (rodIndex == +RailwayMeshGeneratorConfig::SwitchRodIndex::RodHead)
        {
            sourceNodePath = config.getHeadRodNodePath();
        }

        auto rodNode = MetaUnigine::loadNode(sourceNodePath.c_str());
        if (!rodNode)
        {
            error("Can not load rod model from path: %s\n", sourceNodePath.c_str());
            return;
        }

        const Math::Mat4 leftTransform = calcPointRailTransform(true, true, offsetFromRoot,
            scale ? rodNode : MetaUnigine::NodePtr());
        const Math::Mat4 rightTransform = calcPointRailTransform(false, false, offsetFromRoot,
            scale ? rodNode : MetaUnigine::NodePtr());

        auto component = MetaUnigine::getOrAddComponent<RailSwitchPartTransforms>(rodNode);
        assert(component);
        component->setLeftTransform(leftTransform);
        component->setRightTransform(rightTransform);

        rodNode->setName(rodName);
        rodNode->setTransform(initialRightTransform ? rightTransform : leftTransform);
        rodNode->setParent(getMetaObject());
    };

    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);
    const double sleeperWidth =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().sleeperWidth);

    double switchPointDistance = pointRailsLength - (sleeperStep - sleeperWidth) / 2.0;

    // 0. тяга привода
    {
        addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::RodHead, switchPointDistance,
            stringFormat("RodHead%s", suffix).data());
    }

    // 1. Тяга 0 всегда стоит на точке среза
    addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::Rod0, pointRailSlicePoint,
        stringFormat("Rod0%s", suffix).data());

    // 2. Тяга 1 (в сторону корня) ставится в 3м от тяги 0, если до корня больше 6м
    if (pointRailSlicePoint > 6.0)
    {
        addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::Rod1, pointRailSlicePoint - 3.0,
            stringFormat("Rod1%s", suffix).data());
    }

    // 3. Тяга 2 ставится в 3м от тяги 1, если до корня больше 9м
    if (pointRailSlicePoint > 9.0)
    {
        addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::Rod2, pointRailSlicePoint - 6.0,
            stringFormat("Rod2%s", suffix).data());
    }

    // 4. Тяга - 1 ставится ровно между тягой 0 и тягой привода, если от тяги 0 до тяги привода
    // больше  6м
    double sliceLength = pointRailsLength - pointRailSlicePoint;
    if (sliceLength > 6.0)
    {
        double offsetFromRoot = pointRailSlicePoint + (sliceLength * 0.5);
        addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::RodMinusOne, offsetFromRoot,
            stringFormat("RodMinusOne%s", suffix).data());
    }

    //коробка стрелочного перевода.
    auto switchBoxNodeInfo = m_config->getSwitchRodsData(+switchRodConfigType).getSwitchBoxNodeInfo();
    sharedData.switchBoxSides[+switchRodConfigType] = switchBoxNodeInfo.isLeft;
    const char* switchBoxNodePath = switchBoxNodeInfo.nodePath.data();
    if (auto switchBoxNode = MetaUnigine::loadNode(switchBoxNodePath))
    {
        const auto transform = calcPointRailTransform(true, false, switchPointDistance);

        switchBoxNode->setName("AutomaticSwitchBox");
        switchBoxNode->setTransform(transform);
        switchBoxNode->setParent(getMetaObject());

        const auto& borderBarPath = m_config->getSwitchRodsData().borderBarPath;
        if (auto borderBarNode = MetaUnigine::loadNode(borderBarPath.data()))
        {
            const auto& sleepersData = m_config->getJunctionSleepersData();

            const double sleeperStep =
                RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.interval);
            const double sleeperWidth =
                RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.sleeperWidth);

            auto offset = sleeperWidth / 2. - sleeperStep * 2.;
            auto barTransform = transform * Math::Mat4(Math::quat(), Math::Vec3(offset, 0., 0.));

            borderBarNode->setName("BorderBar");
            borderBarNode->setTransform(barTransform);
            borderBarNode->setParent(getMetaObject());
        }
    }
}

void
DynamicRailwayJunctionBase::adjustSkinnedPointRailBones(SharedDataArgsBase& sharedData,
    PointRailInfo& arrow, int splineIdx, int joinSplineIdx, TransformSpline::Point& start,
    const TransformSpline::Point& end, bool isLeft, const MetaUnigine::ObjectMeshSkinnedPtr& skinned,
    const TransformSpline::Point& joinedStartPoint, const TransformSpline::Point& joinedEndPoint,
    const Math::Mat4& joinedScaledTransform)
{
    auto mesh = Mesh::create();
    if (!skinned || !skinned->getMesh(mesh))
    {
        error("Unable to getMesh() from skinned pointRail\n");
        return;
    }

    Vector<Math::mat4> boneTransforms;
    if ((mesh->getNumAnimations() < 1) || !mesh->getBoneTransforms(boneTransforms, 0) ||
        boneTransforms.empty())
    {
        error("Unable to getBoneTransforms() from pointRail\n");
        return;
    }

    // Build first spline by trimming input spline, which has proper arc at the point rail end
    double minDistance = std::min(start.distanceOffset, end.distanceOffset);
    double maxDistance = std::max(start.distanceOffset, end.distanceOffset);
    auto joinedSrcSpline = getSpline(sharedData, splineIdx).createTrimmed(minDistance, maxDistance);
    if (start.distanceOffset > end.distanceOffset)
    {
        joinedSrcSpline = joinedSrcSpline.createReversed();
    }

    Vector<Math::mat4> animFrameTransforms;
    Vector<short> affectedBoneIndices;
    {
        int cutoffIdx = mesh->findBone(PointRailSlicePointBoneName);
        auto invJoinedTransform = Math::inverse(joinedScaledTransform);
        auto rootTransform = joinedScaledTransform * Math::Mat4(boneTransforms[0]);
        for (int boneIdx = 0; boneIdx < mesh->getNumBones(); ++boneIdx)
        {
            auto transform = joinedScaledTransform * Math::Mat4(boneTransforms[boneIdx]);

            auto offset = (transform.getTranslate() - rootTransform.getTranslate()).length();
            if (boneIdx == cutoffIdx)
            {
                arrow.slicePoint = offset;
            }
            offset = joinedSrcSpline.arcLength() - offset;

            auto onSplineTransform = joinedSrcSpline.calcSplineMatrixAtDistance(
                std::clamp(offset, 0.0, joinedSrcSpline.arcLength()));
            if (!onSplineTransform)
            {
                error("Failed to calc spline transform for pointRail bone %d: %s\n", boneIdx);
                continue;
            }
            // Spline has opposite direction (from cutoff to root), so flip it here
            auto rotSplineTransform = *onSplineTransform * Math::Mat4(Math::quat(0.f, 0.f, 180.0f));

            Math::Mat4 bindRotate(boneTransforms[boneIdx].getRotate());
            Math::Mat4 bindScale;
            bindScale.setScale(Math::dvec3(boneTransforms[boneIdx].getScale()));

            animFrameTransforms.push_back(
                Math::mat4(invJoinedTransform * rotSplineTransform * bindRotate * bindScale));
            affectedBoneIndices.push_back(boneIdx);
        }
    }

    int animIdx = mesh->addAnimation("pointRailAnim");
    if (animIdx < 0)
    {
        error("Failed to addAnimation() to pointRail\n");
        return;
    }

    mesh->setNumAnimationFrames(animIdx, 2);
    mesh->setAnimationBones(animIdx, affectedBoneIndices);
    mesh->setAnimationFrame(animIdx, 0, animFrameTransforms);
    mesh->setAnimationFrame(animIdx, 1, animFrameTransforms);

    if (!skinned->setMeshKeepMaterials(mesh))
    {
        error("Failed to setMeshKeepMaterials() to pointRail\n");
        return;
    }

    skinned->setNumLayers(1);
    skinned->setAnimation(0, animIdx);
    skinned->setLayer(0, true, 1.0);
    skinned->setTime(1.0f);
    skinned->setSpeed(0.0f);
    skinned->play();

    arrow.joinedSpline =
        std::shared_ptr<BezierSpline>(new BezierSpline(std::move(joinedSrcSpline)));
}

std::optional<std::reference_wrapper<DynamicRailwayJunctionBase::PointRailInfo>>
DynamicRailwayJunctionBase::generatePointRail(SharedDataArgsBase& sharedData, int splineIdx,
    int joinSplineIdx, TransformSpline::Point& start, const TransformSpline::Point& end,
    bool isLeftType, bool isFront, const char* nodeName, bool scale)
{
    double availableLength = end.distanceOffset - start.distanceOffset;
    int availableLengthMM = RailwayMeshGeneratorConfig::toConfigUnits(std::abs(availableLength));
    int lengthKey =
        m_config->getPointRailsData().chooseSuitablePointRailLength(availableLengthMM, isLeftType);
    if (!lengthKey)
    {
        return std::nullopt;
    }
    auto chosenPointRailLength = RailwayMeshGeneratorConfig::toInnerUnits(lengthKey);
    
    std::string pointRailPath =
        m_config->getPointRailsData().getPointRailPath(lengthKey, isLeftType);
    if (pointRailPath.empty())
    {
        error("Couldn't find point rail model with acceptable length <= %d\n", availableLength);
        return std::nullopt;
    }

    auto pointRailNode = MetaUnigine::loadNode(pointRailPath.data());
    if (!pointRailNode)
    {
        error("Couldn't load point rail model with path %s\n", pointRailPath.data());
        return std::nullopt;
    }

    double xScale = 1.0;
    Math::Mat4 scaleMat;
    if (scale)
    {
        xScale = std::abs(availableLength) / chosenPointRailLength;
        scaleMat.setScale(Math::Vec3(xScale, 1., 1.));
    }
    else
    {
        if (availableLength < 0.)
        {
            start.distanceOffset = end.distanceOffset + chosenPointRailLength;
        }
        else
        {
            start.distanceOffset = end.distanceOffset - chosenPointRailLength;
        }

        auto startTransform =
            getSpline(sharedData, splineIdx).calcSplineMatrixAtDistance(start.distanceOffset);
        if (!startTransform)
        {
            error(
                "Couldn't calculate an updated point rail start position %d spline, %f distance\n",
                splineIdx, end.distanceOffset);
            return std::nullopt;
        }
        start.transform = *startTransform;
    }

    auto joinedStartPoint = getSplineSearch(sharedData, joinSplineIdx)
                           .findClosestPoint(toGlm(start.transform.getTranslate()));
    auto joinedEndPoint = getSplineSearch(sharedData, joinSplineIdx)
                        .findClosestPoint(toGlm(end.transform.getTranslate()));
    if (joinedStartPoint && joinedEndPoint)
    {
        auto& addedNearInfo = sharedData.pointRailsNearBySplineId[joinSplineIdx].emplace_back();
        addedNearInfo = {joinedStartPoint->distanceOffset, joinedEndPoint->distanceOffset, false,
            splineIdx, start.distanceOffset, end.distanceOffset};

        auto dirToPointRail = Math::normalize(
            joinedEndPoint->transform.getTranslate() - end.transform.getTranslate());
        if (Math::cross(joinedEndPoint->transform.getAxisX(), dirToPointRail).z < 0.)
        {
            addedNearInfo.rightSide = true;
        }
    }

    if (!joinedStartPoint)
    {
        joinedStartPoint = start;
    }

    const bool isLeft = isLeftType ^ !isFront;
    const Math::Mat4 joinedTransform = transformFromPositionAndTangent(end.transform.getTranslate(),
        Math::normalize(joinedStartPoint->transform.getTranslate() - end.transform.getTranslate()));

    const Math::Vec3 dirToCenter = Math::cross(joinedStartPoint->transform.getAxisX(),
        isLeft ? Math::Vec3::UP : Math::Vec3::DOWN);
    const Math::Vec3 splittedPosition =
        joinedStartPoint->transform.getTranslate() + dirToCenter * 0.15;
    const Math::Mat4 splittedTransform =
        transformFromPositionAndTangent(end.transform.getTranslate(),
            Math::normalize(splittedPosition - end.transform.getTranslate()));

    auto component = MetaUnigine::getOrAddComponent<RailSwitchPartTransforms>(pointRailNode);
    const auto& leftTransform = (isLeft ? joinedTransform : splittedTransform) * scaleMat;
    component->setLeftTransform(leftTransform);
    const auto& rightTransform = (isLeft ? splittedTransform : joinedTransform) * scaleMat;
    component->setRightTransform(rightTransform);

    pointRailNode->setTransform(leftTransform);
    pointRailNode->setName(makeChildNameUniqueIndexed(getMetaObject(), String(nodeName)));
    getMetaObject()->addChild(pointRailNode);

    if (!m_config->getPointRailsData().materialInfo.name.empty())
    {
        m_setChildNodeMaterialInfo.emplace_back(SetMaterialChildNodeData{pointRailNode->getName(),
            m_config->getPointRailsData().materialInfo});
    }

    std::optional<PointRailInfo>& arrow =
        sharedData.pointRailsBySplineId[splineIdx][isFront ? 0 : 1];
    arrow = {pointRailNode, start, end, leftTransform, rightTransform, isFront, isLeft};
    arrow->joinedStartPoint = joinedStartPoint;

    if (auto slicePointMM = pointRailSlicePoint(lengthKey))
    {
        arrow->slicePoint =
            RailwayMeshGeneratorConfig::toInnerUnits(*slicePointMM) * xScale;
    }
    else
    {
        // Default value for unsupported lengths, most definately will be overrided in next
        // 'adjustSkinnedPointRailBones' call. Placed here to prevent artefacts with invalid models.
        arrow->slicePoint = 1.0;
    }

    if ((pointRailNode->getType() == Node::TYPE::OBJECT_MESH_SKINNED) && joinedStartPoint &&
        joinedEndPoint)
    {
        auto skinnedNode = std::dynamic_pointer_cast<MetaUnigine::ObjectMeshSkinned>(pointRailNode);
        auto joinedScaledTransform = joinedTransform * scaleMat;

        adjustSkinnedPointRailBones(sharedData, *arrow, splineIdx, joinSplineIdx, start, end,
            isLeft, skinnedNode, *joinedStartPoint, *joinedEndPoint, joinedScaledTransform);
    }

    return std::reference_wrapper(*arrow);
}

// Sleepers

std::optional<Unigine::Math::Mat4>
DynamicRailwayJunctionBase::calcSingleSideMountTransform(const SharedDataArgsBase& sharedData,
    int splineIdx, const TransformSpline::Point& point) const
{
    const auto& pointRailsNearIt = sharedData.pointRailsNearBySplineId.find(splineIdx);
    if (pointRailsNearIt == sharedData.pointRailsNearBySplineId.end())
    {
        return std::nullopt;
    }
    for (const auto& pointRailNear : pointRailsNearIt->second)
    {
        double start = std::min(pointRailNear.start, pointRailNear.end);
        double end = std::max(pointRailNear.start, pointRailNear.end);
        if (point.distanceOffset >= start && point.distanceOffset <= end)
        {
            if (!pointRailNear.rightSide)
            {
                return Math::Mat4(Math::quat(0.0f, 0.0f, 180.0f));
            }
            return Math::Mat4::IDENTITY;
        }
    }
    return std::nullopt;
}

std::optional<DynamicRailwayJunctionBase::MountTransform>
DynamicRailwayJunctionBase::calcMountTransform(SharedDataArgsBase& sharedData, int splineIdx,
    const TransformSpline::Point& point) const
{
    auto pointRailsIt = sharedData.pointRailsBySplineId.find(splineIdx);
    if (pointRailsIt != sharedData.pointRailsBySplineId.end())
    {
        if (auto frontArrow = pointRailsIt->second.front())
        {
            double trimStart =
                std::max(frontArrow->end.distanceOffset, frontArrow->start.distanceOffset);
            if (point.distanceOffset <= trimStart)
            {
                return std::nullopt;
            }
        }
        if (auto backArrow = pointRailsIt->second.back())
        {
            double trimEnd =
                std::min(backArrow->start.distanceOffset, backArrow->end.distanceOffset);
            if (point.distanceOffset >= trimEnd)
            {
                return std::nullopt;
            }
        }
    }

    auto frog = getRailFrogAt(sharedData, splineIdx, point.distanceOffset, true);
    if (!frog)
    {
        if (auto addTransform = calcSingleSideMountTransform(sharedData, splineIdx, point))
        {
            return MountTransform{getTransform() * point.transform * *addTransform, true};
        }
        return MountTransform{getTransform() * point.transform};
    }

    if (frog->isObtuse)
    {
        const double mountsOffsetObtuse = RailwayMeshGeneratorConfig::toInnerUnits(
            m_config->getRailFrogData().mountsOffsetObtuse);

        // In the cross point of frog both mounts will not be offseted, to fix it decrease the
        // right side cross point check by tiny offset here
        const double overlapFixM = 0.05;

        bool addOffset = false;
        double mountOffset = mountsOffsetObtuse;
        if (frog->isFlipped)
        {
            addOffset = (splineIdx == frog->rightInputSplineIdx)
                ? (point.distanceOffset <= frog->crossPoint.second.distanceOffset)
                : (point.distanceOffset > frog->crossPoint.first.distanceOffset - overlapFixM);
            mountOffset *= -1.;
        }
        else
        {
            addOffset = (splineIdx == frog->leftInputSplineIdx)
                ? (point.distanceOffset <= frog->crossPoint.first.distanceOffset)
                : (point.distanceOffset > frog->crossPoint.second.distanceOffset - overlapFixM);
        }
        if (addOffset) // Left local side of railFrog
        {
            Math::Mat4 addTransform(Math::quat(), Math::Vec3(0., mountOffset, 0.));
            return MountTransform{getTransform() * point.transform * addTransform};
        }
        else
        {
            return MountTransform{getTransform() * point.transform};
        }
    }
    else
    {
        auto connectionsIt = frog->connections.find(splineIdx);
        if (connectionsIt != frog->connections.end())
        {
            const double mountsOffset =
                RailwayMeshGeneratorConfig::toInnerUnits(m_config->getRailFrogData().mountsOffset);

            // shift leftInputSpline to left, rightInputSpline to right - invert for flipped frog
            bool shiftToLeft = (frog->leftInputSplineIdx == splineIdx) ^ !frog->isFlipped;

            double mountOffset = shiftToLeft ? -mountsOffset : mountsOffset;
            Math::Mat4 addTransform(Math::quat(), Math::Vec3(0., mountOffset, 0.));
            return MountTransform{getTransform() * point.transform * addTransform};
        }
    }
    return std::nullopt;
}

void
DynamicRailwayJunctionBase::doAddMiddleSleepersDefault(SharedDataArgsBase& sharedData, 
    const TransformSpline& middleSpline, double currentDistance,
    double endDistance, double sleeperStep, int type, bool skipFirst, bool skipLast,
    const std::vector<int>& leftOuterSplinesIds, const std::vector<int>& rightOuterSplinesIds,
    const AddMountsFuncs& addMountsFuncs, const AddSleeperInfoFuncType& addSleeperInfoFunc,
    const ModifySleeperWithLengthFuncType& modifySleeperFunc)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();
    const double sleeperStepSwitchBox =
        RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.switchBoxInterval);

    if (type == +SleepersIntervalType::PointRailsFront)
    {
        generateMiddleSplineSleeperDefault(sharedData, middleSpline, currentDistance,
            leftOuterSplinesIds, rightOuterSplinesIds, addSleeperInfoFunc, addMountsFuncs,
            modifySleeperFunc);
        currentDistance += sleeperStepSwitchBox;
    }
    else if (type == +SleepersIntervalType::PointRailsBack)
    {
        generateMiddleSplineSleeperDefault(sharedData, middleSpline, endDistance,
            leftOuterSplinesIds, rightOuterSplinesIds, addSleeperInfoFunc, addMountsFuncs,
            modifySleeperFunc);
        endDistance -= sleeperStepSwitchBox;
    }

    auto genSleeperFunc = [this, &sharedData, &middleSpline, &leftOuterSplinesIds,
                              &rightOuterSplinesIds, &addSleeperInfoFunc, &addMountsFuncs,
                              &modifySleeperFunc](
                              double distance)
    {
        return generateMiddleSplineSleeperDefault(sharedData, middleSpline, distance,
            leftOuterSplinesIds,
            rightOuterSplinesIds, addSleeperInfoFunc, addMountsFuncs,
            modifySleeperFunc);
    };

    generateSleepers(currentDistance, endDistance, sleeperStep, skipFirst, skipLast,
        genSleeperFunc);
}

void
DynamicRailwayJunctionBase::doAddMiddleSleepersSwitchBoxOverride(SharedDataArgsBase& sharedData,
    const TransformSpline& middleSpline, double currentDistance, double endDistance,
    double sleeperStep, int type, bool skipFirst, bool skipLast,
    const std::vector<int>& leftOuterSplinesIds, const std::vector<int>& rightOuterSplinesIds,
    int switchRodsType, bool invertBoxSide,
    const AddMountsFuncs& addMountsFuncs, const AddSleeperInfoFuncType& addSleeperInfoFunc,
    const ModifySleeperWithLengthFuncType& modifySleeperFunc)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();

    const double sleeperStepSwitchBox =
        RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.switchBoxInterval);

    auto addFirstSleepers = [this, &sharedData, &sleepersData, &addSleeperInfoFunc, &middleSpline,
                                &leftOuterSplinesIds, &rightOuterSplinesIds, switchRodsType,
                                invertBoxSide, &addMountsFuncs, &modifySleeperFunc](
                                double currentDistance, double firstStep)
    {
        const double trackWidth =
            RailwayMeshGeneratorConfig::toInnerUnits(m_config->getCommonData().railwayTrackWidth);

        bool turnaroundFirstSleepersOverride = false;
        auto switchBoxSideIt = sharedData.switchBoxSides.find(switchRodsType);
        if ((switchBoxSideIt != sharedData.switchBoxSides.end()) &&
                ((!invertBoxSide && switchBoxSideIt->second) || (invertBoxSide && !switchBoxSideIt->second)))
        {
            turnaroundFirstSleepersOverride = true;
        }

        if (!sleepersData.firstJunctionSleeper.empty())
        {
            auto addFirstSleepersFunc =
                [this, &sharedData, &turnaroundFirstSleepersOverride, &sleepersData,
                    &addSleeperInfoFunc, &modifySleeperFunc, trackWidth](double, const Math::Mat4& transform)
            {
                RailwayMeshGeneratorConfig::MaterialInfo matInfo;
                if (!m_config->getJunctionSleepersData().defaultMaterialInfo.name.empty())
                {
                    matInfo = m_config->getJunctionSleepersData().defaultMaterialInfo;
                }

                const auto& cluster = getOrCreateClusterInfo(sleepersData.firstJunctionSleeper,
                    matInfo, sleepersData.firstJunctionSleeper,
                    m_config->getJunctionSleepersData().visibleDistance);

                auto useTransform =
                    turnaroundFirstSleepersOverride ? transformTurnaroundZ(transform) : transform;

                if (modifySleeperFunc)
                {
                    modifySleeperFunc(trackWidth, useTransform);
                }

                addSleeperInfoFunc(useTransform);
                cluster->transforms.push_back(useTransform);
            };

            generateMiddleSplineSleeper(sharedData, middleSpline, currentDistance,
                leftOuterSplinesIds, rightOuterSplinesIds, addFirstSleepersFunc, addMountsFuncs);
        }
        else
        {
            generateMiddleSplineSleeperDefault(sharedData, middleSpline, currentDistance,
                leftOuterSplinesIds, rightOuterSplinesIds, addSleeperInfoFunc, addMountsFuncs,
                modifySleeperFunc);
        }
        currentDistance += firstStep;

        if (!sleepersData.secondJunctionSleeper.empty())
        {
            auto addSecondSleeperFunc = [this, &sleepersData, &turnaroundFirstSleepersOverride,
                                            &addSleeperInfoFunc, &modifySleeperFunc,
                                            trackWidth]
                (double, const Math::Mat4& transform)
            {
                RailwayMeshGeneratorConfig::MaterialInfo matInfo;
                if (!m_config->getJunctionSleepersData().defaultMaterialInfo.name.empty())
                {
                    matInfo = m_config->getJunctionSleepersData().defaultMaterialInfo;
                }

                const auto& cluster = getOrCreateClusterInfo(sleepersData.secondJunctionSleeper,
                    matInfo, sleepersData.secondJunctionSleeper,
                    m_config->getJunctionSleepersData().visibleDistance);

                auto useTransform =
                    turnaroundFirstSleepersOverride ? transformTurnaroundZ(transform) : transform;

                if (modifySleeperFunc)
                {
                    modifySleeperFunc(trackWidth, useTransform);
                }

                addSleeperInfoFunc(useTransform);
                cluster->transforms.push_back(useTransform);
            };

            generateMiddleSplineSleeper(sharedData, middleSpline, currentDistance,
                leftOuterSplinesIds, rightOuterSplinesIds, addSecondSleeperFunc, addMountsFuncs);
            currentDistance += firstStep;
        }

        return currentDistance;
    };

    if ((type == +SleepersIntervalType::PointRailsFront))
    {
        currentDistance = addFirstSleepers(currentDistance, sleeperStepSwitchBox);
    }
    else if (type == +SleepersIntervalType::PointRailsBack)
    {
        endDistance = addFirstSleepers(endDistance, -sleeperStepSwitchBox);
    }

    auto genSleeperFunc = [this, &sharedData, &middleSpline, &leftOuterSplinesIds,
                              &rightOuterSplinesIds, &addSleeperInfoFunc, &addMountsFuncs,
                              &modifySleeperFunc](
                              double distance)
    {
        return generateMiddleSplineSleeperDefault(sharedData, middleSpline, distance,
            leftOuterSplinesIds, rightOuterSplinesIds, addSleeperInfoFunc, addMountsFuncs,
            modifySleeperFunc);
    };

    generateSleepers(currentDistance, endDistance, sleeperStep, skipFirst, skipLast,
        genSleeperFunc);
}

void
DynamicRailwayJunctionBase::generateMiddleSleepers(SharedDataArgsBase& sharedData, const TransformSpline& middleSpline,
    const std::vector<int>& railSplineIds, const DoAddMiddleSleepersIntervalFuncType& doAddSleepersFunc)
{
    const double trackWidth =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getCommonData().railwayTrackWidth);
    const auto& sleepersData = m_config->getJunctionSleepersData();

    const double sleeperStep = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.interval);
    const double sleeperStepSwitchBox = RailwayMeshGeneratorConfig::toInnerUnits(
        sleepersData.switchBoxInterval);
    const double sleeperWidth = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.sleeperWidth);
    const double guardRailOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getGuardRailsData().offset);

    RailwayMeshGeneratorConfig::MaterialInfo matInfo;
    if (!m_config->getJunctionSleepersData().defaultMaterialInfo.name.empty())
    {
        matInfo = m_config->getJunctionSleepersData().defaultMaterialInfo;
    }

    auto mounts = getOrCreateClusterInfo(sleepersData.mountNodePath, matInfo, "SleeperMounts",
        sleepersData.visibleDistance);
    auto singleSidedMounts = sleepersData.singleSidedMountNodePath.empty() ? mounts
        : getOrCreateClusterInfo(sleepersData.singleSidedMountNodePath, matInfo,
              "SleeperMountsSingleSided", sleepersData.visibleDistance);

    std::string guardRailMountPath =
        m_config->getGuardRailsData().directory + "/" + m_config->getGuardRailsData().mountNode;
    auto guardMounts =
        getOrCreateClusterInfo(guardRailMountPath, RailwayMeshGeneratorConfig::MaterialInfo{},
            "GuardRailMounts", sleepersData.visibleDistance);

    auto mountsFunc = [this, &sharedData, guardRailOffset, &mounts, &singleSidedMounts,
                          &guardMounts, &railSplineIds]
                          (const Math::Mat4& middle, double distance,
                          const SplineNeighbourPoints& left, const SplineNeighbourPoints& right)
    {
        auto glmLeft = toGlm(left.points.first.transform.getTranslate());
        auto glmCenter = toGlm(middle.getTranslate());
        auto glmRight = toGlm(right.points.first.transform.getTranslate());
        for (auto splineIdx : railSplineIds)
        {
            std::optional<TransformSpline::Point> point;
            if (splineIdx == left.splineIdx)
            {
                point = left.points.first;
            }
            else if (splineIdx == right.splineIdx)
            {
                point = right.points.first;
            }
            else
            {
                const auto& splineSearch = sharedData.splineSearchById[splineIdx];
                auto closest = splineSearch.findClosestPoints(glmLeft, glmRight);
                if (!closest || closest->gap > 0.1)
                {
                    continue;
                }
                point = closest->first;
            }

            if (getHeelBlockAt(sharedData, splineIdx, point->distanceOffset))
            {
                continue;
            }

            if (getGuardAt(sharedData, splineIdx, point->distanceOffset, true))
            {
                Math::Mat4 leftMountTransform(Math::quat(), Math::Vec3(0., guardRailOffset, 0.));
                guardMounts->transforms.emplace_back(
                    getTransform() * point->transform * leftMountTransform);
                continue;
            }

            if (getGuardAt(sharedData, splineIdx, point->distanceOffset, false))
            {
                Math::Mat4 rightMountTransform(Math::quat(0.0f, 0.0f, 180.0f),
                    Math::Vec3(0., -guardRailOffset, 0.));
                guardMounts->transforms.emplace_back(
                    getTransform() * point->transform * rightMountTransform);
                continue;
            }

            auto mountTransform = calcMountTransform(sharedData, splineIdx, *point);
            if (!mountTransform)
            {
                continue;
            }

            if (mountTransform->underPointRail)
            {
                singleSidedMounts->transforms.emplace_back(mountTransform->transform);
            }
            else
            {
                mounts->transforms.emplace_back(mountTransform->transform);
            }
        }
    };

    // Split middle sleepers interval into parts for proper first/last sleepers placements
    // SleepersIntervalType is a priority of interval types to select highes in case of overlap

    boost::icl::interval_map<double,  // type of distance
        int,                          // type of value, representation of SleepersIntervalType
        boost::icl::partial_absorber, // default value
        std::less,                    // default value
        boost::icl::inplace_max       // select highest value in case of overlap
        >
        sleepersIntervalMap;

    // add base full middle spline interval with offset = 'sleeperStep / 2.0' from begin/end,
    // first and last sleeper are not skipped
    double startOffset = sleeperStep / 2.0;
    double endOffset = middleSpline.arcLength() - startOffset;
    sleepersIntervalMap.insert({boost::icl::interval<double>::closed(startOffset, endOffset),
        +SleepersIntervalType::Default});

    if (startOffset >= endOffset)
    {
        return;
    }

    // Add all point rails intervals
    for (const auto& [splineIdx, pointRails] : sharedData.pointRailsBySplineId)
    {
        auto railSplineIt = std::find(railSplineIds.begin(), railSplineIds.end(), splineIdx);
        if (railSplineIt == railSplineIds.end())
        {
            continue;
        }

        for (const auto& pointRail : pointRails)
        {
            if (!pointRail)
            {
                continue;
            }

            auto middleStart = middleSpline.findClosestPoint(
                toGlm(pointRail->start.transform.getTranslate()));
            auto middleEnd = middleSpline.findClosestPoint(
                toGlm(pointRail->end.transform.getTranslate()));
            if (middleStart && middleEnd)
            {
                auto distanceFromPointRailStart = (middleStart->transform.getTranslate() -
                    pointRail->start.transform.getTranslate()).length();
                if (distanceFromPointRailStart > trackWidth)
                {
                    continue;
                }

                auto distanceFromPointRailEnd = (middleEnd->transform.getTranslate() -
                    pointRail->end.transform.getTranslate()).length();
                if (distanceFromPointRailEnd > trackWidth)
                {
                    continue;
                }

                // Debug code:
                //if (m_debugVisualizer)
                //{
                //    m_debugVisualizer->addVector(pointRail->start.transform.getTranslate(),
                //        middleStart->transform.getTranslate());
                //    m_debugVisualizer->addVector(pointRail->end.transform.getTranslate(),
                //        middleEnd->transform.getTranslate(), Math::vec4_blue);
                //}

                double diffStart = sleeperWidth / 2.;
                double diffEnd = sleeperStep / 2.;
                if (middleStart->distanceOffset > middleEnd->distanceOffset)
                {
                    diffStart = -diffStart;
                    diffEnd = -diffEnd;
                }
                double start = std::min(middleStart->distanceOffset + diffStart,
                    middleEnd->distanceOffset - diffEnd);
                double end = std::max(middleEnd->distanceOffset - diffEnd,
                    middleStart->distanceOffset + diffStart);

                start = std::clamp(start, startOffset, endOffset);
                end = std::clamp(end, startOffset, endOffset);

                // Add single point rail interval, first and last sleeper are not skipped
                sleepersIntervalMap.add({boost::icl::interval<double>::closed(start, end),
                    pointRail->isFront ? +SleepersIntervalType::PointRailsFront
                                       : +SleepersIntervalType::PointRailsBack});
            }
        }
    }

    // Add all guard rail intervals
    auto addGuardsLambda = [&sharedData, &middleSpline, &sleeperWidth, &sleeperStep, &sleepersIntervalMap](
                               std::map<int, std::list<GuardRailInfo>>& guardRailsBySplineId)
    {
        for (const auto& splineGuards : guardRailsBySplineId)
        {
            for (const auto& guard : splineGuards.second)
            {
                auto glmStart = toGlm(guard.start.transform.getTranslate());
                auto middleStart = middleSpline.findClosestPoint(glmStart);

                auto glmEnd = toGlm(guard.end.transform.getTranslate());
                auto middleEnd = middleSpline.findClosestPoint(glmEnd);

                if (!middleStart || !middleEnd)
                {
                    continue;
                }

                double firstGuardSleeperDistance = middleStart->distanceOffset + sleeperWidth;
                double lastGuardSleeperDistance = middleEnd->distanceOffset - sleeperWidth;

                // Adapt interval to fit sleepers
                const double length =
                    lastGuardSleeperDistance - firstGuardSleeperDistance - sleeperWidth * 2.0;
                int numSleepers(std::abs(std::lround(length / sleeperStep)));
                const double useLength = numSleepers * sleeperStep;
                const double offset = (length - useLength) / 2.0;

                firstGuardSleeperDistance += offset;
                lastGuardSleeperDistance -= offset;

                // Add single guard rail interval, first and last sleeper are not skipped
                sleepersIntervalMap.add({boost::icl::interval<double>::closed(
                                             firstGuardSleeperDistance, lastGuardSleeperDistance),
                    +SleepersIntervalType::GuardRails});
            }
        }
    };
    addGuardsLambda(sharedData.leftGuardRailsBySplineId);
    addGuardsLambda(sharedData.rightGuardRailsBySplineId);

    // generate sleepers from resulting intevals, opened = skip sleeper
    int intervalIdx = 0;
    for (auto const& [interval, type] : sleepersIntervalMap)
    {
        double currentDistance = interval.lower();
        double endDistance = interval.upper();
        bool skipFirst = !boost::icl::is_left_closed(interval.bounds());
        bool skipLast = !boost::icl::is_right_closed(interval.bounds());
        double step = sleeperStep;

        doAddSleepersFunc(currentDistance, endDistance, sleeperStep, type, skipFirst,
            skipLast, {mountsFunc});

        ++intervalIdx;
    }
}

void
DynamicRailwayJunctionBase::generateMiddleSleepers(SharedDataArgsBase& sharedData, const TransformSpline& middleSpline,
    const std::vector<int>& mountSplineIds, const AddSleeperInfoFuncType& addSleeperInfoFunc)
{
    generateMiddleSleepers(sharedData, middleSpline, mountSplineIds,
        [this, &sharedData, &middleSpline, &addSleeperInfoFunc](double startDst, double endDst,
            double step, int type, bool skipFirst, bool skipLast, const AddMountsFuncs& addMountsFuncs)
    {
        doAddMiddleSleepersDefault(sharedData, middleSpline, startDst, endDst, step, type, skipFirst,
            skipLast, sharedData.leftOuterSplinesIds, sharedData.rightOuterSplinesIds,
            addMountsFuncs, addSleeperInfoFunc);
    });
}

bool
DynamicRailwayJunctionBase::generateSleepers(double& currentDistance, double lastDistance, double baseStep, bool skipFirst, bool skipLast,
    const GenerateSleeperFuncType& genSleeperFunc)
{
    const double interval = lastDistance - currentDistance;
    int numSleepers(std::abs(std::lround(interval / baseStep)));
    const double useSleeperStep = (numSleepers != 0) ? (interval / numSleepers) : baseStep;

    double nextDistance = currentDistance + useSleeperStep;
    if (!skipFirst)
    {
        ++numSleepers;
        nextDistance = currentDistance;
    }

    if (skipLast)
    {
        --numSleepers;
    }

    int countGenerated = 0;
    for (int i = 0; i < numSleepers; i++)
    {
        if (genSleeperFunc(nextDistance))
        {
            ++countGenerated;
        }
        currentDistance = nextDistance;
        nextDistance += useSleeperStep;
    }

    return (countGenerated > 0);
}

bool
DynamicRailwayJunctionBase::generateMiddleSplineSleeper(SharedDataArgsBase& sharedData,
    const TransformSpline& middleSpline, double distance,
    const std::vector<int> leftOuterSplinesIds, const std::vector<int> rightOuterSplinesIds,
    const AddSleeperWithLengthFuncType& addSleepersFunc,
    const AddMountsFuncs& addMountsFuncs)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();

    auto middleTransform = middleSpline.getTransformAtDistance(distance);
    if (!middleTransform)
    {
        return false;
    }

    const double maxSleeperLength =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().maxLength);
    const double scanDistance = maxSleeperLength / 2.;
    const auto glmPosition = toGlm(middleTransform->getTranslate());

    const auto glmCenter = toGlm(middleTransform->getTranslate());

    const auto glmLeft = toGlm(*middleTransform * Math::Vec3(0., scanDistance, 0.));
    auto leftOuterPoint =
        findFirstNearestSplinePoint(sharedData, leftOuterSplinesIds, glmLeft, glmCenter);

    // Debug code:
    /*if (m_debugVisualizer)
    {
        m_debugVisualizer->addVector(fromGlm(glmLeft), fromGlm(glmCenter));
        if (leftOuterPoint)
        {
            m_debugVisualizer->addPoint(leftOuterPoint->points.first.transform.getTranslate());
            m_debugVisualizer->addPoint(leftOuterPoint->points.second.transform.getTranslate());
        }
    }*/

    const auto glmRight = toGlm(*middleTransform * Math::Vec3(0., -scanDistance, 0.));
    auto rightOuterPoint = findFirstNearestSplinePoint(sharedData, rightOuterSplinesIds,
        glmRight, glmCenter);

    // Debug code:
    /*if (m_debugVisualizer)
    {
         m_debugVisualizer->addPoint(middleTransform->getTranslate(),
             Math::vec4::WHITE);

        m_debugVisualizer->addVector(fromGlm(glmRight), fromGlm(glmCenter), Math::vec4::BLUE);
        if (rightOuterPoint)
        {
            m_debugVisualizer->addPoint(rightOuterPoint->points.first.transform.getTranslate(),
                Math::vec4::BLUE);
            m_debugVisualizer->addPoint(rightOuterPoint->points.second.transform.getTranslate(),
                Math::vec4::BLUE);
        }
    }*/

    if (!leftOuterPoint || leftOuterPoint->points.gap > 0.1 || !rightOuterPoint ||
        rightOuterPoint->points.gap > 0.1)
    {
        return false;
    }

    double outerRailsDistance = (rightOuterPoint->points.first.transform.getTranslate() -
        leftOuterPoint->points.first.transform.getTranslate())
                                    .length();
    if (outerRailsDistance > maxSleeperLength)
    {
        return false;
    }

    // минимальная ширина выступа от оси рамного рельса до конца бруса,
    // чтобы влезло рельсовое скрепление.
    const double minSleeperOuterGap = RailwayMeshGeneratorConfig::toInnerUnits(550) * 2.0;
    double desiredLength = outerRailsDistance + minSleeperOuterGap;

    // TODO: add param to disable this
    auto useMiddleTransform = Math::lerp(leftOuterPoint->points.first.transform,
        rightOuterPoint->points.first.transform, 0.5);

    addSleepersFunc(desiredLength, m_transform * useMiddleTransform);

    if (!sharedData.firstSleeperTransform ||
        sharedData.firstSleeperTransform->distanceOffset > distance)
    {
        sharedData.firstSleeperTransform = {distance, useMiddleTransform};
    }
    if (!sharedData.lastSleeperTransform ||
        sharedData.lastSleeperTransform->distanceOffset < distance)
    {
        sharedData.lastSleeperTransform = {distance, useMiddleTransform};
    }

    for (const auto& addMountsFunc : addMountsFuncs)
    {
        addMountsFunc(useMiddleTransform, distance, *leftOuterPoint, *rightOuterPoint);
    }

    return true;
}

bool
DynamicRailwayJunctionBase::generateMiddleSplineSleeperDefault(SharedDataArgsBase& sharedData,
    const TransformSpline& middleSpline, double distance,
    const std::vector<int> leftOuterSplinesIds, const std::vector<int> rightOuterSplinesIds,
    const AddSleeperInfoFuncType& addSleeperInfoFunc,
    const AddMountsFuncs& addMountsFuncs,
    const ModifySleeperWithLengthFuncType& modifySleeperFunc)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();

    auto getStandartSleeperLength = [this, &sleepersData](double length)
    {
        const int minLength = sleepersData.minLength;
        const int lengthStep = sleepersData.lengthIncreaseStep;

        int lengthInMM = std::max(RailwayMeshGeneratorConfig::toConfigUnits(length), minLength);
        int numSteps = (lengthInMM - minLength) / lengthStep;
        int tail = (lengthInMM - minLength) % lengthStep;
        int standartLength = minLength + (numSteps * lengthStep);
        if (tail > 0)
        {
            standartLength += lengthStep;
        }

        return standartLength;
    };

    auto getClusterMeshPath = [this, &sleepersData](int standartLength) -> std::string
    {
        std::string namePatten = sleepersData.nodeNamePattern;
        int asteriskPos = namePatten.find_first_of('*', 0);

        std::stringstream ss;
        ss << sleepersData.directory << "/";
        ss << namePatten.substr(0, asteriskPos);
        ss << standartLength;
        ss << namePatten.substr(asteriskPos + 1);

        return ss.str();
    };

    auto getSleepersClusterByLength = [this, &sleepersData, &getStandartSleeperLength, &getClusterMeshPath]
    (double& length) -> const ClusterInfoPtr&
    {
        int key = getStandartSleeperLength(length);
        length = RailwayMeshGeneratorConfig::toInnerUnits(key);
        std::string sourceModelPath = getClusterMeshPath(key);

        RailwayMeshGeneratorConfig::MaterialInfo matInfo;
        if (!sleepersData.defaultMaterialInfo.name.empty())
        {
            matInfo = sleepersData.defaultMaterialInfo;
        }

        std::string name = "sleepers_" + std::to_string(key);
        return getOrCreateClusterInfo(sourceModelPath, matInfo, name,
            m_config->getJunctionSleepersData().visibleDistance);
    };

    auto addSleeperFunc =
        [this, &sharedData, &sleepersData, &getSleepersClusterByLength, &modifySleeperFunc,
            &addSleeperInfoFunc](
                              double desiredLength,
                              const Math::Mat4& middleTransform)
    {
        auto& sleepers = getSleepersClusterByLength(desiredLength);

        Math::Mat4 useMiddleTransform(middleTransform);
        if (m_config->getJunctionSleepersData().addRandomAngle)
        {
            useMiddleTransform =
                addRandomTransform(sharedData, sleepersData.randomAngle, middleTransform);
        }

        if (modifySleeperFunc)
        {
            modifySleeperFunc(desiredLength, useMiddleTransform);
        }

        sleepers->transforms.push_back(useMiddleTransform);

        if (addSleeperInfoFunc)
        {
            addSleeperInfoFunc(sleepers->transforms.back());
        }
    };

    return generateMiddleSplineSleeper(sharedData, middleSpline, distance,
        leftOuterSplinesIds,
        rightOuterSplinesIds,
        addSleeperFunc,
        addMountsFuncs);
}

void
DynamicRailwayJunctionBase::generateSleepersOnOut(SharedDataArgsBase& sharedData, int splineIdx,
    const Math::Mat4& startMiddleTransform, double endDistance,
    const AddSleeperInfoFuncType& addSleeperInfoFunc)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();

    const double sleeperLengthHalf =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getCommonData().railwayTrackWidth);
    const double sleeperWidth = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.sleeperWidth);
    const double sleeperStep = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.interval);
    const double sleeperMaxLengthHalf =
        RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.maxLength) / 2.0;
    const int numSleeperVariants = m_config->getBranchSleepersData().variantsCount;
    if (numSleeperVariants == 0)
    {
        return;
    }

    auto getClusterKey = [numSleeperVariants]()
    {
        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, numSleeperVariants - 1);

        return dist(rd);
    };

    auto getClusterPathByKey = [this](int key)
    {
        std::string namePatten = m_config->getBranchSleepersData().nodeNamePattern;
        int asteriskPos = namePatten.find_first_of('*', 0);

        std::stringstream ss;
        ss << m_config->getBranchSleepersData().directory << "/";
        ss << namePatten.substr(0, asteriskPos);
        ss << key;
        ss << namePatten.substr(asteriskPos + 1);

        return ss.str();
    };

    auto getRandomSleeperCluster = [this, &sharedData, &getClusterKey,
                                       &getClusterPathByKey]() -> const ClusterInfoPtr&
    {
        int key = getClusterKey();
        std::string sourceModelPath = getClusterPathByKey(key);

        RailwayMeshGeneratorConfig::MaterialInfo matInfo;
        if (!m_config->getBranchSleepersData().defaultMaterialInfo.name.empty())
        {
            matInfo = m_config->getBranchSleepersData().defaultMaterialInfo;
        }

        std::string name = "Sleepers_V" + std::to_string(key);
        return getOrCreateClusterInfo(sourceModelPath, matInfo, name,
            m_config->getJunctionSleepersData().visibleDistance);
    };

    auto addSleeper = [this, &addSleeperInfoFunc, &getRandomSleeperCluster](
                          const Math::Mat4& transform)
    {
        auto cluster = getRandomSleeperCluster();
        cluster->transforms.push_back(getTransform() * transform);

        addSleeperInfoFunc(transform);
    };

    const auto& spline = getSpline(sharedData, splineIdx);
    if (endDistance < 0.0)
    {
        endDistance = spline.arcLength() - endDistance;
    }

    const auto glmLeft = toGlm(startMiddleTransform * Math::Vec3(0., sleeperMaxLengthHalf, 0.));
    const auto glmRight = toGlm(startMiddleTransform * Math::Vec3(0., -sleeperMaxLengthHalf, 0.));

    // Debug code:
    //if (m_debugVisualizer)
    //{
    //    m_debugVisualizer->addSpline(spline);
    //    m_debugVisualizer->addPoint(startMiddleTransform.getTranslate());
    //    m_debugVisualizer->addVector(fromGlm(glmLeft), fromGlm(glmRight));
    //}

    auto sleepersStartPoint =
        getSplineSearch(sharedData, splineIdx).findClosestPoints(glmLeft, glmRight);
    if (sleepersStartPoint)
    {
        // Debug code:
        //if (m_debugVisualizer)
        //{
        //    m_debugVisualizer->addPoint(sleepersStartPoint->first.transform.getTranslate());
        //}

        const auto tangent1 = sleepersStartPoint->first.transform.getAxisX();
        const auto tangent2 = sleepersStartPoint->second.transform.getAxisX();

        const double angle = std::abs(wrapValueAround(normalizedAngleFromToRad(tangent1, tangent2),
            -Math::Consts::PI05, Math::Consts::PI05));
        double distanceOffset = sleeperWidth * 2.;
        if (std::abs(angle - Math::Consts::PI05) > 0.01)
        {
            distanceOffset += sleeperLengthHalf / tan(angle);
        }

        double distance = endDistance - sleepersStartPoint->first.distanceOffset;
        double absDistanceWithExtents = std::abs(distance) - distanceOffset;
        const int numSleepers(int(absDistanceWithExtents / sleeperStep));
        if (numSleepers > 0)
        {
            const double useSleeperStep = absDistanceWithExtents / numSleepers;
            double currentDistance = std::min(sleepersStartPoint->first.distanceOffset +
                    std::copysign(1.0, distance) * distanceOffset, endDistance);
            for (int i = 0; i < numSleepers + 1; i++)
            {
                auto sleeperTransform = spline.calcSplineMatrixAtDistance(currentDistance);
                if (sleeperTransform)
                {
                    if (sleepersData.addRandomAngle)
                    {
                        addSleeper(addRandomTransform(sharedData, sleepersData.randomAngle,
                            *sleeperTransform));
                    }
                    else
                    {
                        addSleeper(*sleeperTransform);
                    }
                }
                currentDistance += useSleeperStep;
            }
        }
    }
}

// Surfaces

void DynamicRailwayJunctionBase::generateRailSurfaces(SharedDataArgsBase& sharedData,
    GenerateMeshFromSplineArgs& args)
{
    const int numLODs = 2;

    std::string surfaceName = args.surfaceName;
    for (int i = 0; i < numLODs; i++)
    {
        std::stringstream ss;
        ss << surfaceName << "LOD" << i;
        args.surfaceName = ss.str();

        int idx = (+RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::RailsLOD0) + i;
        auto& genData = m_config->getSurfaceData(idx);
        generateSurface(sharedData, genData, args);
    }
}

bool DynamicRailwayJunctionBase::generateSurface(SharedDataArgsBase& sharedData,
    RailwayMeshGeneratorConfig::SurfaceGenerationData& config,
    GenerateMeshFromSplineArgs& args)
{
    if (!config.pointProfile || config.pointProfile->points.empty())
    {
        return false;
    }

    args.profilePointPositions = config.pointProfile->points;
    args.profilePointNormals = config.pointProfile->normals;
    args.profilePointUVCoords = config.pointProfile->uvCoords;
    args.numSegmentsPerUnit = config.numSegmentsPerUnit;
    args.uCoordStepPerUnit = config.texCoordVStep;
    if (args.preCap || args.postCap)
    {
        args.profileCapTris = config.getCapTriangles();
    }

    generateMeshFromSpline(args);

    sharedData.outSurfaces.emplace_back(OutSurfaceData{ args.surfaceName, config });
    return true;
}

// Materials

void DynamicRailwayJunctionBase::setupMeshMaterials(SharedDataArgsBase& sharedData)
{
    message("setup mesh materials...\n");

    for (auto const& surface : sharedData.outSurfaces)
    {
        const char* surfaceName = surface.surfaceName.c_str();
        surface.genData.applySurfaceParams(m_object, surfaceName);
    }

    for (auto const& railFrog : sharedData.railFrogsInfo)
    {
        if (railFrog->object)
        {
            setupMaterialsForRailFrog(sharedData, *railFrog, m_object);
        }
    }
}

void
DynamicRailwayJunctionBase::setupMaterialsForRailFrog(
    SharedDataArgsBase& sharedData, const RailFrogInfo& railFrog,
    MetaUnigine::ObjectMeshStaticPtr const& junctionMesh)
{
    auto railFrogMesh = railFrog.object;
    auto leftInputRail = sharedData.railInfoById[railFrog.leftInputSplineIdx];
    auto rightInputRail = sharedData.railInfoById[railFrog.rightInputSplineIdx];

    auto leftFrontRailLOD0 = leftInputRail.surfaceName + "LOD0";
    auto leftBackRailLOD0 = rightInputRail.surfaceName + "LOD0";
    auto leftFrontRailLOD1 = leftInputRail.surfaceName + "LOD1";
    auto leftBackRailLOD1 = rightInputRail.surfaceName + "LOD1";

    auto rightFrontRailLOD0 = rightInputRail.surfaceName + "LOD0";
    auto rightBackRailLOD0 = leftInputRail.surfaceName + "LOD0";
    auto rightFrontRailLOD1 = rightInputRail.surfaceName + "LOD1";
    auto rightBackRailLOD1 = leftInputRail.surfaceName + "LOD1";

    std::unordered_multimap<std::string, std::string> srcSurfacesLookup
    {
        {"rail_r_lod0", rightBackRailLOD0},
        {"rail_r_lod0", rightFrontRailLOD0},
        {"rail_r_lod1", rightBackRailLOD1},
        {"rail_r_lod1", rightFrontRailLOD1},
        {"rail_l_lod0", leftBackRailLOD0},
        {"rail_l_lod0", leftFrontRailLOD0},
        {"rail_l_lod1", leftBackRailLOD1},
        {"rail_l_lod1", leftFrontRailLOD1}
    };

    for (int i = 0, num = railFrogMesh->getNumSurfaces(); i < num; i++)
    {
        std::string dstSurfaceName = railFrogMesh->getSurfaceName(i);
        std::transform(dstSurfaceName.begin(), dstSurfaceName.end(), dstSurfaceName.begin(),
            [](unsigned char c) { return std::tolower(c); });
        auto range = srcSurfacesLookup.equal_range(dstSurfaceName);

        for (auto it = range.first; it != range.second; it++)
        {
            std::string srcSurfaceName = it->second;
            int srcSurfIdx = junctionMesh->findSurface(srcSurfaceName.c_str());
            if (srcSurfIdx >= 0)
            {
                std::string materialName = junctionMesh->getMaterialName(srcSurfIdx);
                railFrogMesh->setMaterial(materialName.c_str(), i);
                break;
            }
        }//for (auto it = range.first; it != range.second; it++)
    }//for (int i = 0, num = railFrogMesh->getNumSurfaces(); i < num; i++)

    auto getSurfaceName = [](char suffix, int lod)
    {
        std::stringstream ss;
        ss << "rail_" << suffix << "_LOD" << lod;

        return ss.str();
    };

    auto getMaterialName = [&getSurfaceName, &railFrogMesh](char suffix, int lod)
        -> std::string
    {
        std::string surfaceName = getSurfaceName(suffix, lod);
        int idx = railFrogMesh->findSurface(surfaceName.c_str());
        if (idx >= 0)
        {
            return railFrogMesh->getMaterialName(idx);
        }

        return std::string{};
    };

    const int k_numLODS = 2;
    for (int i = 0; i < k_numLODS; i++)
    {
        std::string leftMaterial = getMaterialName('l', i);
        std::string rightMaterial = getMaterialName('r', i);

        if (leftMaterial == rightMaterial)
        {
            std::string surfaceName = getSurfaceName('c', i);
            int idx = railFrogMesh->findSurface(surfaceName.c_str());
            if (idx >= 0)
            {
                railFrogMesh->setMaterial(leftMaterial.c_str(), idx);
            }
        }
    }//for (int i = 0; i < k_numLODS; i++)
}

std::optional<DynamicRailwayJunctionBase::SplineNeighbourPoints>
DynamicRailwayJunctionBase::findFirstNearestSplinePoint(SharedDataArgsBase& sharedData,
    const std::vector<int>& splineIds, const glm::dvec3& from, const glm::dvec3& to,
    double maxDistance)
{
    std::optional<SplineNeighbourPoints> resultPoint;
    for (const auto& splineIdx : splineIds)
    {
        const auto& splineSearch = sharedData.splineSearchById[splineIdx];
        auto splinePoint = splineSearch.findClosestPoints(from, to);
        if (splinePoint && (!resultPoint || ((splinePoint->gap < maxDistance) &&
                    (splinePoint->gap < resultPoint->points.gap))))
        {
            resultPoint = SplineNeighbourPoints{ splineIdx, *splinePoint };
        }
    }
    return resultPoint;
}

std::optional<TransformSpline::NeighbourPoints>
DynamicRailwayJunctionBase::findNearestSplinePoint(SharedDataArgsBase& sharedData,
    const std::vector<int>& splineIds, const glm::dvec3& from, const glm::dvec3& to)
{
    std::optional<TransformSpline::NeighbourPoints> resultPoint;
    for (const auto& splineIdx : splineIds)
    {
        auto splinePoint = sharedData.splineSearchById[splineIdx].findClosestPoints(
            from, to);
        if (!resultPoint || (splinePoint->gap < resultPoint->gap))
        {
            resultPoint = splinePoint;
        }
    }
    return resultPoint;
}

void
DynamicRailwayJunctionBase::applyMaterialTextures(const Unigine::NodePtr& node)
{
    auto staticMesh = checked_ptr_cast<ObjectMeshStatic>(node);
    if (!staticMesh)
    {
        return;
    }

    for (auto& tmpMatInfo : m_temporaryMaterialsInfo)
    {
        const char* surfaceName = tmpMatInfo.surfaceName.c_str();
        tmpMatInfo.genData.applySurfaceParams(staticMesh, surfaceName, tmpMatInfo.addMaterialInfo);
    }
}

void
DynamicRailwayJunctionBase::swapSplines(int firstIdx, int secondIdx)
{
    std::swap(m_splines[firstIdx], m_splines[secondIdx]);
}

std::vector<int>
DynamicRailwayJunctionBase::interFillVec(int first, int last)
{
    int signedSize = last - first;
    std::vector<int> result;
    result.reserve(std::abs(signedSize));

    int step = std::copysign(1, signedSize);
    for (int value = first;; value += step)
    {
        result.push_back(value);
        if (value == last)
        {
            break;
        }
    }
    return result;
}

std::vector<int>
DynamicRailwayJunctionBase::uniteVecs(const std::vector<int>& first, const std::vector<int>& last)
{
    std::vector<int> result;
    result.reserve(first.size() + last.size());
    result.insert(result.begin(), first.begin(), first.end());
    result.insert(result.begin(), last.begin(), last.end());
    return result;
}
