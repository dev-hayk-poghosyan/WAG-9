#include "DynamicRailwayBranchSegment.h"
#include "RailwayMeshGeneratorConstants.h"

#include "neokon-shared/CalcOffsetedBezierSegment.h"
#include "neokon-shared/GenerateMeshFromSplineList.h"
#include "neokon-shared/GlmUnigineInterop.h"

#include <UnigineNodes.h>
#include <UnigineWorld.h>
#include <UnigineMaterials.h>

#include <random>

using namespace Unigine;

namespace
{
const char* detailStateName = "detail";
const char* detailTexPrefix = "detail_";
const char* detailMaskUvBlendParamName = "detail_mask_uv_transform";

const double blendEmbankmentBias = 0.005;
const double minimalSplineProfileOffset2 = 0.005;
}//Anonymous namespace

const char* DynamicRailwayBranchSegment::EmbankmentSurfaceName = "embankment";
const char* DynamicRailwayBranchSegment::RailsSurfaceNameLOD0 = "railsLOD0";
const char* DynamicRailwayBranchSegment::RailsSurfaceNameLOD1 = "railsLOD1";
const double DynamicRailwayBranchSegment::SleepersPerTexture = 8; // In one texture artists draw 8 sleepers

void DynamicRailwayBranchSegment::createAndAppendSplittedSegments(
    const std::function<void(const DynamicRailwayPartPtr&)>& appendFunc, uint32_t branchId,
    uint16_t segmentIdx, const std::string& configPath,
    const SplineNeighborConfigs& neighborConfigPaths, double startDistanceM, bool isLast,
    const BezierSegment& bezierInWorld, const boost::uuids::uuid& pathUUID,
    const CheckAlignedBoxWorldVisibilityFunc& checkVisibilityFunc)
{
    if (checkVisibilityFunc && !checkVisibilityFunc(bezierInWorld.getAABB()))
    {
        return;
    }

    auto config = getOrLoadRailwayMeshGeneratorConfig(configPath);
    if (!config)
    {
        return;
    }
    const double splitStep = config->getBranchCommonData().maxSegmentLength;

    const double fullLength = bezierInWorld.arcLength();
    const uint16_t subSegmentsCount(std::clamp(std::lround(fullLength / splitStep), 1l, 65535l));
    const double distanceStep = fullLength / subSegmentsCount;
    for (uint16_t subSegment = 0; subSegment < subSegmentsCount; ++subSegment)
    {
        const bool subSegmentIsLast = (subSegment == subSegmentsCount - 1);

        const double fromDistanceM = subSegment * distanceStep;
        const double toDistanceM = subSegmentIsLast ? fullLength : (subSegment + 1) * distanceStep;

        BezierSegment bezierWorldPart;
        if (subSegmentsCount > 1)
        {
            bezierWorldPart = bezierInWorld.subSegmentDistanceToDistance(fromDistanceM, toDistanceM);
        }
        else
        {
            bezierWorldPart = bezierInWorld;
        }

        if (checkVisibilityFunc && !checkVisibilityFunc(bezierWorldPart.getAABB()))
        {
            continue;
        }

        const uint32_t subId = segmentIdx + (subSegment << 16);
        SplineNeighborConfigs segmentNeighborConfigPaths;
        segmentNeighborConfigPaths.previous = configPath;
        if (subSegment == 0)
        {
            segmentNeighborConfigPaths.previous = neighborConfigPaths.previous;
        }
        segmentNeighborConfigPaths.next = configPath;
        if (subSegment == subSegmentsCount - 1)
        {
            segmentNeighborConfigPaths.next = neighborConfigPaths.next;
        }

        appendFunc(DynamicRailwayBranchSegment::create(branchId, subId, configPath,
            segmentNeighborConfigPaths, startDistanceM + fromDistanceM, isLast && subSegmentIsLast,
            bezierWorldPart, pathUUID, config));
    }
}

DynamicRailwayBranchSegmentPtr
DynamicRailwayBranchSegment::create(uint32_t branchId, uint32_t segmentIdx,
    const std::string& configPath, const SplineNeighborConfigs& neighborConfigPaths,
    double startDistanceM, bool isLast, const BezierSegment& bezierInWorld,
    const boost::uuids::uuid& pathUUID, const RailwayMeshGeneratorConfigPtr& config)
{
    DynamicRailwayBranchSegmentPtr segment(new DynamicRailwayBranchSegment(branchId, segmentIdx,
        configPath, neighborConfigPaths, startDistanceM, isLast, bezierInWorld, config));

    std::vector<DynamicRailwayPartWithPaths::Path> paths{DynamicRailwayPartWithPaths::Path{pathUUID,
        startDistanceM, startDistanceM + bezierInWorld.arcLength(), bezierInWorld, segment}};
    segment->setPaths(std::move(paths));

    return segment;
}

DynamicRailwayBranchSegment::DynamicRailwayBranchSegment(uint32_t branchId, uint32_t segmentIdx,
    const std::string& configPath, const SplineNeighborConfigs& neighborConfigPaths,
    double startDistanceM, bool isLast, const BezierSegment& bezierInWorld,
    const RailwayMeshGeneratorConfigPtr& config)
    : DynamicRailwayPartGenerated(branchId, segmentIdx, configPath, config)
    , m_neighborConfigPaths(neighborConfigPaths)
    , m_startDistanceM(startDistanceM)
    , m_segmentIsLast(isLast)
{
    setBBox(fromGlm(bezierInWorld.getAABB().minCoords()),
        fromGlm(bezierInWorld.getAABB().maxCoords()));

    m_bezierSegment = BezierSegment(
        bezierInWorld.p0() - bezierInWorld.p0(), bezierInWorld.p1() - bezierInWorld.p0(),
        bezierInWorld.p2() - bezierInWorld.p0(), bezierInWorld.p3() - bezierInWorld.p0()
    );

    m_segmentTransform.setTranslate((fromGlm(bezierInWorld.p0())));
}

bool DynamicRailwayBranchSegment::load()
{
    if (!loadConfig())
    {
        return false;
    }

    const auto& commonConfig = m_config->getBranchCommonData();
    const auto& embankmentConfig = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);
    auto& railConfigLOD0 = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::RailsLOD0);
    auto& railConfigLOD1 = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::RailsLOD1);
    const auto& sleepersConfig = m_config->getBranchSleepersData();

    if (!railConfigLOD0.pointProfile || railConfigLOD0.pointProfile->points.empty())
    {
        error("Empty railConfigLOD0.pointProfile for \"%s\" meshgen", m_configPath.data());
        return false;
    }

    m_mesh = Mesh::create();

    const double metersPerFullTexture = 1. / embankmentConfig.texCoordVStep;
    const double preSleepersStep = metersPerFullTexture / SleepersPerTexture;
    const double segmentLength = m_bezierSegment.arcLength();

    double stepsCount = segmentLength / preSleepersStep;
    int stepsIntegerCount = int(stepsCount);
    double left = std::fmod(stepsCount, 1.);
    if (left >= 0.5)
    {
        ++stepsIntegerCount;
    }
    m_sleepersStepM = segmentLength / stepsIntegerCount;
    m_uvCorrection = preSleepersStep / m_sleepersStepM;

    auto const& railProfileLOD0 = railConfigLOD0.pointProfile->points;
    double minProfileY = std::numeric_limits<double>::max();
    double maxProfileY = std::numeric_limits<double>::lowest();
    for (auto const& facePointCoords : railProfileLOD0)
    {
        if (facePointCoords.y > maxProfileY)
        {
            maxProfileY = facePointCoords.y;
        }

        if (facePointCoords.y < minProfileY)
        {
            minProfileY = facePointCoords.y;
        }
    }
    const double profileWidth = maxProfileY - minProfileY;
    const double halfProfileWidth = profileWidth * 0.5;
    m_railOffsetM = (RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getCommonData().railwayTrackWidth) + halfProfileWidth) * 0.5;

    if (embankmentConfig.pointProfile && !embankmentConfig.pointProfile->points.empty() &&
        isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::BranchEmbankment))
    {
        generateEmbankment(embankmentConfig);
        generateCaps(embankmentConfig, m_bezierSegment);
    }

    if (isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::BranchRails))
    {
        m_fishPlatePositions = generateRails(RailsSurfaceNameLOD0, railConfigLOD0,
            commonConfig.gapStep, commonConfig.gapLength);
        generateRails(RailsSurfaceNameLOD1, railConfigLOD1);
    }

    if (isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::BranchAdditional))
    {
        generateOtherSurfaces();
    }

    m_mesh->createBounds();

    if (sleepersConfig.variantsCount > 0)
    {
        m_sleeperPositions.resize(sleepersConfig.variantsCount);
        DynamicRailwayPartWithSleepers::SleepersPtr sleepersInfo(
            new DynamicRailwayPartWithSleepers::Sleepers);

        std::mt19937 random(getKey().id + getKey().subId);
        std::uniform_int_distribution<int> flipAngle(0, 1);
        std::uniform_real_distribution<double> addAngle(-sleepersConfig.randomAngle,
            sleepersConfig.randomAngle);
        std::uniform_int_distribution<int> randomIndex(0, sleepersConfig.variantsCount - 1);

        auto addTransform = [this, &sleepersInfo, &random, &flipAngle, &addAngle, &randomIndex, &sleepersConfig]
        (const Math::Mat4& splinePose)
        {
            auto transform = m_segmentTransform * splinePose;

            if (sleepersConfig.addRandomAngle)
            {
                double angle = addAngle(random);
                if (flipAngle(random) == 1)
                {
                    angle += 180.;
                }
                Math::Mat4 addRotate;
                addRotate.setRotate(Math::Vec3::UP, angle);
                transform *= addRotate;
            }

            int variantIndex = randomIndex(random);
            m_sleeperPositions[variantIndex].push_back(transform);

            sleepersInfo->push_back(DynamicRailwayPartWithSleepers::Sleeper{transform});
        };

        double startDistance = 0.;// For branch aligned integer sleepers count use 'm_startDistance'
        generateSleeperTransforms(m_bezierSegment, m_sleepersStepM / 2., m_sleepersStepM,
            m_segmentIsLast, startDistance, addTransform);

        std::unique_lock<std::mutex> lock(m_sleepersInfosProtect);
        m_sleepersInfos.push_back(sleepersInfo);
    }

    return true;
}

void DynamicRailwayBranchSegment::unload()
{
    m_mesh.clear();
    m_sleeperPositions.clear();
    m_fishPlatePositions.clear();
    m_capInfos.clear();
    onUnload();
}

DynamicRailwayBranchSegment::CapInfo&
DynamicRailwayBranchSegment::addCapInfo(const std::string& capNodePath,
    const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo)
{
    m_capInfos.emplace_back(CapInfo{capNodePath, materialInfo});
    return m_capInfos.back();
}

void
DynamicRailwayBranchSegment::applyBlendEmbankment(const Unigine::ObjectMeshStaticPtr& object)
{
    const auto& blendMatInfo = m_config->getCommonData().blendEmbankmentMaterialInfo;
    if (blendMatInfo.name.empty())
    {
        return;
    }

    if(!m_neighborConfigPaths.previous || !m_neighborConfigPaths.next ||
        *m_neighborConfigPaths.previous == *m_neighborConfigPaths.next)
    {
        return;
    }

    auto baseMat = Materials::findMaterial(blendMatInfo.name.data());
    if (!baseMat)
    {
        return;
    }

    auto prevCfg = getOrLoadRailwayMeshGeneratorConfig(*m_neighborConfigPaths.previous);
    auto nextCfg = getOrLoadRailwayMeshGeneratorConfig(*m_neighborConfigPaths.next);
    if (!prevCfg || !nextCfg)
    {
        return;
    }
    const auto& prevEmbankment =
        prevCfg->getSurfaceData(+RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);
    const auto& nextEmbankment =
        nextCfg->getSurfaceData(+RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);

    if (prevEmbankment.defaultMaterialInfo.textures.empty() &&
        nextEmbankment.defaultMaterialInfo.textures.empty())
    {
        return;
    }

    String matName(blendMatInfo.name.data());
    matName += "_";
    matName += blendMatInfo.generateMatNameFromParams();
    matName += "_";
    matName += prevEmbankment.defaultMaterialInfo.generateMatNameFromParams();
    matName += "_";
    matName += nextEmbankment.defaultMaterialInfo.generateMatNameFromParams();

    const double metersPerFullTexture = 1. / prevEmbankment.texCoordVStep;

    // Substract bias to fix gap at the end, caused by scale calculation numeric inaccuracy
    const double uScale =
        (metersPerFullTexture / m_bezierSegment.arcLength()) - blendEmbankmentBias;

    matName += String::format("_%.3f", uScale);

    auto temporaryBlendMat = Materials::findMaterial(matName);
    if (!temporaryBlendMat)
    {
        temporaryBlendMat = baseMat->inherit(matName);
        if (!temporaryBlendMat)
        {
            return;
        }

        int paramIdx = temporaryBlendMat->findParameter(detailMaskUvBlendParamName);
        if (paramIdx < 0)
        {
            return;
        }
        auto uvTransform = temporaryBlendMat->getParameterFloat4(paramIdx);
        uvTransform.x = uScale;
        temporaryBlendMat->setParameterFloat4(paramIdx, uvTransform);

        prevEmbankment.defaultMaterialInfo.applyTextures(temporaryBlendMat, detailTexPrefix);
        nextEmbankment.defaultMaterialInfo.applyTextures(temporaryBlendMat);

        blendMatInfo.apply(temporaryBlendMat);
        m_config->getCommonData().sleepersMaterialInfo.apply(temporaryBlendMat);
    }

    int surIdx = object->findSurface(EmbankmentSurfaceName);
    if (surIdx < 0)
    {
        return;
    }
    object->setMaterial(temporaryBlendMat, surIdx);
}

NodePtr DynamicRailwayBranchSegment::createNodes()
{
    if (!m_mesh)
    {
        return {};
    }

    auto result = ObjectMeshStatic::create(m_mesh);
    result->setName(String::format("branch_%d_segment_%d[%d]", m_key.id, m_key.subId & 0xFFFF,
        m_key.subId >> 16));

    applyObjectParameters(result);

    const auto& embankmentConfig =
        m_config->getSurfaceData(+RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);
    if (!embankmentConfig.capNodePath.empty())
    {
        for (const auto& capInfo : m_capInfos)
        {
            for (const auto& capTransform : capInfo.transforms)
            {
                auto capNode = World::loadNode(capInfo.capNodePath.data());
                if (!capNode)
                {
                    error("Can not load cap node: %s\n", capInfo.capNodePath.data());
                    break;
                }
                auto capObject = checked_ptr_cast<ObjectMeshStatic>(capNode);
                if (!capObject)
                {
                    error("Can not cast cap node: %s\n", capInfo.capNodePath.data());
                    capNode.deleteForce();
                    break;
                }
                capObject->setParent(result);
                capObject->setTransform(capTransform);
                capObject->setName("cap");
                capObject->setMaterial(capInfo.materialInfo.getOrCreateMaterial(), 0);
            }
        }
    }

    applyBlendEmbankment(result);

    onGenerationFinished(result);


#ifdef DYNAMIC_RAILWAYS_DEBUG
    if (m_debugVisualizer)
    {
        m_debugVisualizer->visualize(m_segmentTransform);
    }
#endif

    return result;
}

bool DynamicRailwayBranchSegment::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto segment = std::dynamic_pointer_cast<DynamicRailwayBranchSegment>(old);
    if (!segment)
    {
        return true;
    }
    if (segment.get() != this)
    {
        return true;
    }
    return DynamicRailwayPartGenerated::isDifferentWith(old);
}

void DynamicRailwayBranchSegment::validateClusters(const GetOrCreateClusterFunc& getOrCreateCluster,
    const std::optional<Math::Vec3>& center) const
{
    if (!m_sleeperPositions.empty() &&
        isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::BranchSleepers))
    {
        const auto& sleepersConfig = m_config->getBranchSleepersData();
        for (int variant = 0; variant < sleepersConfig.variantsCount; ++variant)
        {
            auto nodePath = sleepersConfig.getTemplatePath(variant);
            RailwayMeshGeneratorConfig::MaterialInfo matInfo;
            if (!sleepersConfig.defaultMaterialInfo.name.empty())
            {
                matInfo = sleepersConfig.defaultMaterialInfo;
            }

            if (auto cluster = getOrCreateCluster(nodePath, matInfo))
            {
                float visDistance = cluster->getVisibleDistance() * 2.0f; // For fast moving * 2.0f

                auto invClusterTransform = Math::inverse(cluster->getWorldTransform());
                for (const auto& transform : m_sleeperPositions[variant])
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

    if (!m_fishPlatePositions.empty() &&
        isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::BranchFishPlates))
    {
        const auto& commonConfig = m_config->getBranchCommonData();
        if (!commonConfig.fishPlateNode.empty())
        {
            if (auto cluster = getOrCreateCluster(commonConfig.fishPlateNode,
                    /*matInfo=*/commonConfig.fishPlateMaterialInfo))
            {
                float visDistance = cluster->getVisibleDistance() * 2.0f; // For fast moving * 2.0f

                const Math::Mat4 invClusterTransform = Math::inverse(cluster->getWorldTransform());
                const Math::Mat4 leftTransform(Math::quat_identity,
                    Math::Vec3(0, m_railOffsetM, 0));
                const Math::Mat4 rightTransform(Math::quat(0, 0, 180),
                    Math::Vec3(0, -m_railOffsetM, 0));
                for (const auto& transform : m_fishPlatePositions)
                {
                    if (!center || (*center - transform.getTranslate()).length() < visDistance)
                    {
                        auto preTranform = invClusterTransform * transform;

                        cluster->setMeshTransform(cluster->addMeshTransform(),
                            Math::mat4(preTranform * leftTransform));

                        cluster->setMeshTransform(cluster->addMeshTransform(),
                            Math::mat4(preTranform * rightTransform));
                    }
                }
            }
        }
    }
}

DynamicRailwayPartWithSleepers::ConstSleepersPtr
DynamicRailwayBranchSegment::getSleepers(int pathIdx) const
{
    std::unique_lock<std::mutex> lock(m_sleepersInfosProtect);
    if (m_sleepersInfos.empty())
    {
        return nullptr;
    }
    return m_sleepersInfos[pathIdx];
}

double
DynamicRailwayBranchSegment::generateSleeperTransforms(const BezierSegment& segment,
    double pathShift, double pathStep, bool trimEnd, double distance,
    const std::function<void(const Unigine::Math::Mat4&)>& addTransform)
{
    const double pathLength = segment.arcLength();
    const double startStep = (distance - pathShift) / pathStep;
    const double startStepOffset = std::fmod(1. - std::fmod(startStep, 1.), 1.);
    const double startPathOffset = startStepOffset * pathStep;

    double pathOffset = startStepOffset * pathStep;
    for (; pathOffset < pathLength; pathOffset += pathStep)
    {
        if (trimEnd && (pathLength - pathOffset < pathStep / 3.))
        {
            break;
        }
        addTransform(calcBezierSegmentDistanceMatrix(segment, pathOffset));
    }

    return startPathOffset;
}

void
DynamicRailwayBranchSegment::generateEmbankment(
    const RailwayMeshGeneratorConfig::SurfaceGenerationData& config)
{
    if (!config.pointProfile || config.pointProfile->points.empty())
    {
        return;
    }

    GenerateMeshFromSplineArgs genArgs;

    genArgs.dstMesh = m_mesh;
    genArgs.profilePointPositions = config.pointProfile->points;
    genArgs.profilePointNormals = config.pointProfile->normals;
    genArgs.profilePointUVCoords = config.pointProfile->uvCoords;
    genArgs.numSegmentsPerUnit = config.numSegmentsPerUnit;
    genArgs.uCoordStepPerUnit = config.texCoordVStep * m_uvCorrection;
    genArgs.surfaceIndex = getOrCreateMeshSurface(EmbankmentSurfaceName);
    genArgs.splineList.push_back(m_bezierSegment);
    genArgs.adaptiveDividing = true;

    genArgs.lengthOffsetForUV = 0.; // For branch aligned integer sleepers count use 'm_startDistance'
    generateMeshFromSpline(genArgs);
}

Vector<Math::Mat4> DynamicRailwayBranchSegment::generateRails(const char* surfaceName,
    RailwayMeshGeneratorConfig::SurfaceGenerationData& config, double gapStepM, double gapLengthM)
{
    if (!config.pointProfile || config.pointProfile->points.empty())
    {
        return {};
    }

    GenerateMeshFromSplineArgs genArgs;

    genArgs.dstMesh = m_mesh;
    genArgs.profilePointPositions = config.pointProfile->points;
    genArgs.profilePointNormals = config.pointProfile->normals;
    genArgs.profilePointUVCoords = config.pointProfile->uvCoords;
    genArgs.numSegmentsPerUnit = config.numSegmentsPerUnit;
    genArgs.uCoordStepPerUnit = config.texCoordVStep;
    genArgs.surfaceIndex = getOrCreateMeshSurface(surfaceName);
    genArgs.splineList.push_back(m_bezierSegment);
    genArgs.adaptiveDividing = true;

    if (gapStepM <= 0) // No gaps
    {
        genArgs.profileOffset.y = m_railOffsetM;
        genArgs.profileMirrorY = false;
        generateMeshFromSpline(genArgs);

        genArgs.profileOffset.y = -m_railOffsetM;
        genArgs.profileMirrorY = true;
        generateMeshFromSpline(genArgs);

        return {};
    }

    genArgs.profileCapTris = config.getCapTriangles();
    const double arcLength = m_bezierSegment.arcLength();
    const int numCapVerts = genArgs.profilePointPositions.size();

    auto validateGapDistance = [this, &genArgs](double distance)
    {
        const double minSleeperCapDistance = m_sleepersStepM / 2.;

        // Valid only for segment aligned integer sleepers count
        double prevSleeperCapDistance = std::fmod(distance - m_sleepersStepM / 2., m_sleepersStepM);
        if (prevSleeperCapDistance < minSleeperCapDistance)
        {
            return distance + minSleeperCapDistance - prevSleeperCapDistance;
        }

        double nextSleeperCapDistance = m_sleepersStepM - prevSleeperCapDistance;
        if (nextSleeperCapDistance < minSleeperCapDistance)
        {
            return distance - minSleeperCapDistance + nextSleeperCapDistance;
        }

        return distance;
    };

    Vector<Math::Mat4> gapPositions;

    int firstRailSegmentIndex = int(m_startDistanceM / gapStepM);
    int lastRailSegmentIndex = int((m_startDistanceM + arcLength) / gapStepM);
    for (int railSegmentIndex = firstRailSegmentIndex; railSegmentIndex <= lastRailSegmentIndex;
        ++railSegmentIndex)
    {
        // TODO: assign 'genArgs.minOffset' for rails UV tiling?
        genArgs.preCap = false;
        genArgs.postCap = false;

        genArgs.cropLengthBegin = railSegmentIndex * gapStepM - m_startDistanceM;
        if (genArgs.cropLengthBegin <= 0.)
        {
            genArgs.cropLengthBegin = 0.;
            if (!m_neighborConfigPaths.previous || m_neighborConfigPaths.previous->empty())
            {
                genArgs.preCap = true;
            }
        }
        else
        {
            genArgs.preCap = true;
            genArgs.cropLengthBegin = validateGapDistance(genArgs.cropLengthBegin);

            // Calculate transform 10 cm forward along the rail to get its position and
            // calculate the rail direction between this point and gap position.
            const auto afterGap =
                calcBezierSegmentDistanceMatrix(m_bezierSegment, genArgs.cropLengthBegin + 0.1);
            const auto atGap =
                calcBezierSegmentDistanceMatrix(m_bezierSegment, genArgs.cropLengthBegin);

            Math::vec3 directionAtGap(atGap.getTranslate() - afterGap.getTranslate());
            directionAtGap.normalize();
            const Math::vec3 right = Math::cross(Math::vec3_up, directionAtGap);

            const Math::mat3 rotationAtGap(directionAtGap, right, Math::cross(directionAtGap, right));
            const Math::Mat4 gapInSegmentTransform(rotationAtGap, atGap.getTranslate());

            gapPositions.emplace_back(m_segmentTransform * gapInSegmentTransform);
        }

        genArgs.cropLengthEnd = (railSegmentIndex + 1) * gapStepM - m_startDistanceM;
        if (genArgs.cropLengthEnd + 1. > arcLength) // "+1." to prevent fish plate not fit
        {
            genArgs.cropLengthEnd = arcLength;
            ++railSegmentIndex; // skip next segment due to appending it to current one

            if (!m_neighborConfigPaths.next || m_neighborConfigPaths.next->empty())
            {
                genArgs.postCap = true;
            }
        }
        else
        {
            genArgs.postCap = true;
            genArgs.cropLengthEnd = validateGapDistance(genArgs.cropLengthEnd);
            genArgs.cropLengthEnd -= gapLengthM;
        }

        genArgs.profileOffset.y = m_railOffsetM;
        genArgs.profileMirrorY = false;
        generateMeshFromSpline(genArgs);

        genArgs.profileOffset.y = -m_railOffsetM;
        genArgs.profileMirrorY = true;
        generateMeshFromSpline(genArgs);
    }

    return gapPositions;
}

void DynamicRailwayBranchSegment::generateOtherSurfaces()
{
    const size_t surfacesCount = m_config->getSurfacesData().size();
    if (surfacesCount > +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::Total)
    {
        GenerateMeshFromSplineArgs genArgs;
        genArgs.dstMesh = m_mesh;
        genArgs.adaptiveDividing = true;

        for (int surfaceIndex = +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::Total;
            surfaceIndex < surfacesCount; ++surfaceIndex)
        {
            const auto& config = m_config->getSurfaceData(surfaceIndex);
            if (!config.pointProfile || config.pointProfile->points.empty())
            {
                continue;
            }

            genArgs.splineList.clear();
            if (config.profileOffset.length2() > minimalSplineProfileOffset2)
            {
                genArgs.splineList.push_back(
                    calcOffsetedBezierSegment(m_bezierSegment, toGlm(config.profileOffset)));
            }
            else
            {
                genArgs.splineList.push_back(m_bezierSegment);
            }

            genArgs.profilePointPositions = config.pointProfile->points;
            genArgs.profilePointNormals = config.pointProfile->normals;
            genArgs.profilePointUVCoords = config.pointProfile->uvCoords;
            genArgs.numSegmentsPerUnit = config.numSegmentsPerUnit;
            genArgs.uCoordStepPerUnit = config.texCoordVStep;
            genArgs.surfaceIndex = getOrCreateMeshSurface(config.name.data());

            generateMeshFromSpline(genArgs);
            generateCaps(config, genArgs.splineList.front());
        }
    }
}

void
DynamicRailwayBranchSegment::generateCaps(
    const RailwayMeshGeneratorConfig::SurfaceGenerationData& config, const BezierSegment& segment)
{
    if (config.capNodePath.empty())
    {
        return;
    }

    auto& capInfo = addCapInfo(config.capNodePath, config.defaultMaterialInfo);
    if (m_neighborConfigPaths.previous)
    {
        auto prevConfig = getOrLoadRailwayMeshGeneratorConfig(*m_neighborConfigPaths.previous);
        if (!prevConfig || hasDifferentEmbankmentProfile(prevConfig))
        {
            capInfo.transforms.push_back(
                calcSplinePointMatrix(segment, 0.0, TransformOrthogonalization::Off) *
                Math::Mat4(Math::quat(0., 0., 180.)));
        }
    }

    if (m_neighborConfigPaths.next)
    {
        auto nextConfig = getOrLoadRailwayMeshGeneratorConfig(*m_neighborConfigPaths.next);
        if (!nextConfig || hasDifferentEmbankmentProfile(nextConfig))
        {
            capInfo.transforms.push_back(
                calcSplinePointMatrix(segment, 1.0, TransformOrthogonalization::Off));
        }
    }


}

int DynamicRailwayBranchSegment::getOrCreateMeshSurface(const char* name)
{
    int surfaceIndex = m_mesh->findSurface(name);
    if (surfaceIndex >= 0)
    {
        return surfaceIndex;
    }
    return m_mesh->addSurface(name);
}
