#include "DynamicRailwayAdjunct.h"

#include "neokon-shared/GenerateMeshFromSplineList.h"
#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/TransformTurnaroundZ.h"
#include "neokon-shared/FloatingPointCompare.h"

using namespace Unigine;
//----------------------------------------------------------------------------------
// class DynamicContactRail
//----------------------------------------------------------------------------------
const char* DynamicRailwayAdjunct::ContactRailsSurfaceNameLOD0 = "contactRailLOD0";
const char* DynamicRailwayAdjunct::ContactRailsSurfaceNameLOD1 = "contactRailLOD1";

DynamicRailwayAdjunctPtr
DynamicRailwayAdjunct::create(uint32_t partId, uint32_t segmentIdx, const std::string& configPath,
    double startDistanceM, double endDistanceM, const BezierSpline& bezierInWorld,
    double adjunctFullLengthM, double adjunctStartOffsetM, bool lastSegment,
    const DynamicRailwayPartWithSleepers::ConstSleepersPtr& sleepers)
{
    return std::make_shared<DynamicRailwayAdjunct>(partId, segmentIdx, configPath, startDistanceM,
        endDistanceM, bezierInWorld, adjunctFullLengthM, adjunctStartOffsetM, lastSegment,
        sleepers);
}

DynamicRailwayAdjunct::DynamicRailwayAdjunct(uint32_t partId, uint32_t segmentIndex,
    const std::string& meshGetConfigPath, double startDistanceM, double endDistanceM,
    const BezierSpline& bezierInWorld, double adjunctFullLengthM, double adjunctStartOffsetM,
    bool lastSegment, const DynamicRailwayPartWithSleepers::ConstSleepersPtr& sleepers)
    : DynamicRailwayPathBaseAdjunct(partId, segmentIndex, meshGetConfigPath, startDistanceM,
          endDistanceM, bezierInWorld, CmDoubleCompare::equal(adjunctStartOffsetM, 0.0),
          lastSegment, sleepers)
    , m_adjunctStartOffsetM(adjunctStartOffsetM)
    , m_adjunctFullLengthM(adjunctFullLengthM)
{
}

double
calcDistance(Math::Vec3 p1, Math::Vec3 p2)
{
    const Math::Vec3 offsettedToOriginP2 = p2 - p1;

    const double distance = offsettedToOriginP2.length();

    return distance;
}

bool DynamicRailwayAdjunct::load()
{
    if (!loadConfig())
    {
        return false;
    }

    m_mesh = Mesh::create();

    std::optional<Unigine::Math::Mat4> nodeTransform;
    for (const auto& part : m_config->getAdjunctData().parts)
    {
        if (part.enabled)
        {
            loadPart(part, nodeTransform);
        }
    }
    if (!nodeTransform.has_value())
    {
        return false;
    }
    m_nodeTransform = *nodeTransform;
    m_mesh->createBounds();

    return nodeTransform.has_value();
}

bool DynamicRailwayAdjunct::loadPart(const RailwayMeshGeneratorConfig::AdjunctPartData& part,
    std::optional<Unigine::Math::Mat4>& nodeTransform)
{
    const auto offsetVec = getOffsetVector(part);
    const Math::Mat4 offsetMat(Math::quat_identity, offsetVec);
    const double firstBraceDistanceOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(part.firstBraceDistanceOffsetMm);
    const double secondBraceDistanceOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(part.secondBraceDistanceOffsetMm);
    const double penultBraceDistanceOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(part.penultBraceDistanceOffsetMm);
    const double lastBraceDistanceOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(part.lastBraceDistanceOffsetMm);
    const double braceGapHalf = RailwayMeshGeneratorConfig::toInnerUnits(part.braceGapMm) / 2.;
    const double frontDistanceOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(part.frontDistanceOffsetMm);
    const double backDistanceOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(part.backDistanceOffsetMm);

    auto addClusterNode = [this](const std::string& nodePath, const Math::Mat4& transform)
    {
        auto nodeTransformsIt = m_nodesWithTransforms.find(nodePath);
        if (nodeTransformsIt == m_nodesWithTransforms.end())
        {
            nodeTransformsIt =
                m_nodesWithTransforms.emplace(nodePath, std::vector<Unigine::Math::Mat4>()).first;
        }
        auto& nodeTransforms = nodeTransformsIt->second;
        nodeTransforms.push_back(transform);
    };

    if (m_firstSegment && part.enableFrontCap)
    {
        if (auto frontCapTransform =
                m_srcSpline.calcSplineMatrixAtDistance(m_startDistanceM + frontDistanceOffset))
        {
            addClusterNode(part.frontCapNodePath, *frontCapTransform * offsetMat);
        }
    }

    if (m_lastSegment && part.enableBackCap)
    {
        if (auto backCapTransform = m_srcSpline.calcSplineMatrixAtDistance(
                m_endDistanceM - backDistanceOffset))
        {
            addClusterNode(part.backCapNodePath,
                *backCapTransform * offsetMat * Math::Mat4(Math::quat(0., 0., 180.)));
        }
    }

    if (m_endDistanceM - backDistanceOffset <= m_startDistanceM + frontDistanceOffset)
    {
        return true;
    }

    // Don't forget, that offseted spline has different length, than srcSpline
    const auto srcSpline = m_srcSpline.createTrimmed(m_startDistanceM, m_endDistanceM);
    const auto offsetedSpline = srcSpline.createOffseted(toGlm(offsetVec));
    const auto localSrcSpline = offsetedSpline.createLocalized(nodeTransform);
    if (!nodeTransform)
    {
        return false;
    }

    const double srcLength = srcSpline.arcLength();
    const double offsetedLength = offsetedSpline.arcLength();

    TransformSpline srcTransformSpline;
    srcTransformSpline.build(srcSpline);

    TransformSpline offsetedTransformSpline;
    offsetedTransformSpline.build(offsetedSpline);

    GenerateMeshFromSplineArgs genArgs;
    genArgs.dstMesh = m_mesh;
    genArgs.splineList = localSrcSpline.buildSegmentsVector();
    genArgs.profileMirrorY = isLeftSide(part);
    genArgs.preCap = true;
    genArgs.postCap = true;

    auto generateRailSurface = [this, &genArgs](const char* surfaceName,
                                   RailwayMeshGeneratorConfig::SurfaceGenerationData& config,
                                   double startDistance, double endDistance)
    {
        genArgs.surfaceIndex = getOrCreateMeshSurface(surfaceName);
        genArgs.profilePointPositions = config.pointProfile->points;
        genArgs.profilePointNormals = config.pointProfile->normals;
        genArgs.profilePointUVCoords = config.pointProfile->uvCoords;
        genArgs.numSegmentsPerUnit = config.numSegmentsPerUnit;
        genArgs.uCoordStepPerUnit = config.texCoordVStep;
        genArgs.cropLengthBegin = startDistance;
        genArgs.cropLengthEnd = endDistance;

        if (genArgs.preCap || genArgs.postCap)
        {
            genArgs.profileCapTris = config.getCapTriangles();
        }

        generateMeshFromSpline(genArgs);
    };

    auto genRails = [&](double startDistance, double endDistance)
    {
        if (startDistance >= endDistance)
        {
            return;
        }

        for (const auto& surface : part.surfaces)
        {
            generateRailSurface(surface->name.data(), *surface, startDistance, endDistance);
        }
    };

    if (!part.enableBraces || !part.braceGapMm || (part.braceIntervalMm <= 0))
    {
        genRails(m_firstSegment ? frontDistanceOffset : 0.0,
            m_lastSegment ? offsetedLength - backDistanceOffset : offsetedLength);
    }

    if (!part.enableBraces || part.braceNodePath.empty() || (part.braceIntervalMm <= 0))
    {
        return true;
    }

    auto genRailSegment = [&part, &genRails](double startDistance, double endDistance)
    {
        if (!part.braceGapMm)
        {
            return;
        }
        genRails(startDistance, endDistance);
    };

    struct OffsetedSplinePoint
    {
        double distance{0.0};
        Unigine::Math::Mat4 transform;
    };
    std::function<std::optional<OffsetedSplinePoint>(double)> findSplinePoint;

    std::map<double, OffsetedSplinePoint> sleeperBounds;
    if (part.fitBracesToSleepers && m_sleepers)
    {
        for (const auto& sleeper : *m_sleepers)
        {
            const auto& sleeperTransform = sleeper.worldTransform;

            auto sleeperCenter = toGlm(sleeperTransform.getTranslate());

            // 'sleeperTransform' can be randomly rotated 180 degree by the UP for better visual
            // quality, so check the offset to both directions as workaround.
            const Math::Mat4 offsetMat1(Math::quat_identity, offsetVec);
            auto sleeperEdge1 = toGlm((sleeperTransform * offsetMat1).getTranslate());
            const Math::Mat4 offsetMat2(Math::quat_identity,
                Math::Vec3(offsetVec.x, -offsetVec.y, offsetVec.z));
            auto sleeperEdge2 = toGlm((sleeperTransform * offsetMat2).getTranslate());

            // srcToSleeper is for railway central spline.
            auto srcToSleeper = srcTransformSpline.findClosestPoint(sleeperCenter);
            // offsetedToSleeper is for adjunct own offseted spline.
            auto offsetedToSleeper =
                offsetedTransformSpline.findClosestPoints(sleeperEdge1, sleeperEdge2);
            if (srcToSleeper && offsetedToSleeper)
            {
                sleeperBounds.emplace(srcToSleeper->distanceOffset,
                    OffsetedSplinePoint{offsetedToSleeper->first.distanceOffset,
                        offsetedToSleeper->first.transform});
            }
        }
        if (sleeperBounds.empty())
        {
            return true;
        }

        double maxDistance = 0.5;
        findSplinePoint = [&part, &offsetedSpline, &sleeperBounds, maxDistance](
                              double distance) -> std::optional<OffsetedSplinePoint>
        {
            auto sleeperIt = sleeperBounds.lower_bound(distance);
            if (sleeperIt == sleeperBounds.end())
            {
                sleeperIt = std::prev(sleeperIt);
            }
            else
            {
                auto prevIt = std::prev(sleeperIt);
                if (prevIt != sleeperBounds.end())
                {
                    if (std::abs(prevIt->first - distance) < std::abs(sleeperIt->first - distance))
                    {
                        sleeperIt = prevIt;
                    }
                }
            }
            if ((sleeperIt != sleeperBounds.end()) &&
                std::abs(sleeperIt->first - distance) > maxDistance)
            {
                sleeperIt = sleeperBounds.end();
            }
            if (sleeperIt != sleeperBounds.end())
            {
                if (part.braceSleeperOffsetMm != 0)
                {
                    const double braceSleeperOffset =
                        RailwayMeshGeneratorConfig::toInnerUnits(part.braceSleeperOffsetMm);

                    double newDistance = sleeperIt->second.distance - braceSleeperOffset;
                    if (auto transformOpt = offsetedSpline.calcSplineMatrixAtDistance(newDistance))
                    {
                        return OffsetedSplinePoint{newDistance, transformOpt.value()};
                    }
                }
                return sleeperIt->second;
            }
            return std::nullopt;
        };
    }
    else
    {
        findSplinePoint = [&offsetedSpline](
                              double distance) -> std::optional<OffsetedSplinePoint>
        {
            auto transformOpt = offsetedSpline.calcSplineMatrixAtDistance(distance);
            if (transformOpt.has_value())
            {
                return OffsetedSplinePoint{distance, transformOpt.value()};
            }
            return std::nullopt;
        };
    }

    // Input distances (m_adjunctFullLengthM, m_adjunctStartOffsetM) are bound to src spline,
    // not the offseted one. 
    const double bracesFullDistance = m_adjunctFullLengthM - frontDistanceOffset -
        backDistanceOffset - firstBraceDistanceOffset - lastBraceDistanceOffset -
        secondBraceDistanceOffset - penultBraceDistanceOffset;
    const double startBraceOffsetM = m_adjunctStartOffsetM - firstBraceDistanceOffset -
        secondBraceDistanceOffset - frontDistanceOffset;

    double braceInterval = RailwayMeshGeneratorConfig::toInnerUnits(part.braceIntervalMm);
    int numBraces = (int)std::round(bracesFullDistance / braceInterval);
    if (numBraces > 0)
    {
        braceInterval = bracesFullDistance / numBraces;
    }
    else
    {
        braceInterval = bracesFullDistance;
    }

    int firstBraceIdx = std::max((int)std::ceil(startBraceOffsetM / braceInterval), 0);

    double lastBraceDistance = startBraceOffsetM + srcLength;
    if (m_lastSegment)
    {
        lastBraceDistance -= penultBraceDistanceOffset;
        lastBraceDistance -= lastBraceDistanceOffset;
        lastBraceDistance -= backDistanceOffset;

        // Fix possible missing last brace due to float calculations
        lastBraceDistance += braceInterval * 0.1;
    }

    int lastBraceIdx = std::max((int)std::floor(lastBraceDistance / braceInterval), 0);

    auto braceNodeTransformsIt = m_nodesWithTransforms.find(part.braceNodePath);
    if (braceNodeTransformsIt == m_nodesWithTransforms.end())
    {
        braceNodeTransformsIt =
            m_nodesWithTransforms.emplace(part.braceNodePath, std::vector<Unigine::Math::Mat4>())
                .first;
    }
    auto& braceNodeTransforms = braceNodeTransformsIt->second;

    // prevDistance is in adjunct segment local space, not adjunct space
    double prevDistance = m_firstSegment ? frontDistanceOffset : 0.0;
    auto addBrace = [this, &braceNodeTransforms, &part, &findSplinePoint, &genRailSegment,
                        &prevDistance, &braceGapHalf]
    (double distance) -> std::optional<Math::Mat4>
    {
        auto splinePointOpt = findSplinePoint(distance - m_adjunctStartOffsetM);
        if (splinePointOpt)
        {
            // splinePointDistance is in adjunct segment local space, not adjunct space
            double splinePointDistance = splinePointOpt->distance;
            const auto& splinePointTransform = splinePointOpt->transform;

            if (part.rotateBraces && isLeftSide(part))
            {
                Math::Mat4 rotatedTransform = transformTurnaroundZ(splinePointTransform);

                braceNodeTransforms.push_back(rotatedTransform);
            }
            else
            {
                braceNodeTransforms.push_back(splinePointTransform);
            }
            genRailSegment(prevDistance, splinePointDistance - braceGapHalf);
            prevDistance = splinePointDistance + braceGapHalf;
            return braceNodeTransforms.back();
        }
        return std::nullopt;
    };

    if (part.secondBraceDistanceOffsetMm > 0)
    {
        const double firstBraceDistance = frontDistanceOffset + firstBraceDistanceOffset;
        if ((firstBraceDistance >= m_adjunctStartOffsetM) &&
            (firstBraceDistance <= m_adjunctStartOffsetM + srcLength))
        {
            addBrace(firstBraceDistance);
        }
    }

    for (int braceIdx = firstBraceIdx; braceIdx <= lastBraceIdx; ++braceIdx)
    {
        // currentDistance is in adjunct local space, not adjunct segment space
        double currentDistance =
            firstBraceDistanceOffset + secondBraceDistanceOffset + frontDistanceOffset +
            braceIdx * braceInterval;

        auto braceTransformOpt = addBrace(currentDistance);
#ifdef DYNAMIC_RAILWAYS_DEBUG
        if (m_debugVisualizer && braceTransformOpt)
        {
            m_debugVisualizer->addMesage(String::format("%d %f", braceIdx, currentDistance),
                braceTransformOpt->getTranslate());
        }
#endif
    }

    if (part.penultBraceDistanceOffsetMm > 0)
    {
        const double lastBraceDistance = m_adjunctFullLengthM - lastBraceDistanceOffset -
            backDistanceOffset;
        if ((lastBraceDistance >= m_adjunctStartOffsetM) &&
            (lastBraceDistance <= m_adjunctStartOffsetM + srcLength))
        {
            addBrace(lastBraceDistance);
        }
    }

    const double endDistance = m_lastSegment ? offsetedLength - backDistanceOffset : offsetedLength;
    genRailSegment(prevDistance, endDistance);

    return true;
}

void
DynamicRailwayAdjunct::unload()
{
    m_nodesWithTransforms.clear();

    DynamicRailwayPathBaseAdjunct::unload();
}

NodePtr
DynamicRailwayAdjunct::createNodes()
{
    ObjectMeshStaticPtr result;

    if (m_mesh)
    {
        result = doCreateNode(String::format("guard_rail_%d_segment_%d[%d]", m_key.id,
            m_key.subId & 0xFFFF, m_key.subId >> 16));

        for (const auto& part : m_config->getAdjunctData().parts)
        {
            for (auto& surface : part.surfaces)
            {
                surface->applySurfaceParams(result, surface->name.data());
            }
        }

        onGenerationFinished(result);
    }

#ifdef DYNAMIC_RAILWAYS_DEBUG
    if (m_debugVisualizer)
    {
        m_debugVisualizer->visualize({});
    }
#endif

    return result;
}

bool
DynamicRailwayAdjunct::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto oldContactRail = dynamic_cast<DynamicRailwayAdjunct*>(old.get());
    if (!oldContactRail)
    {
        return true;
    }

    return DynamicRailwayPathBaseAdjunct::isDifferentWith(old);
}

void
DynamicRailwayAdjunct::validateClusters(GetOrCreateClusterFunc const& getOrCreateCluster,
                                          std::optional<Unigine::Math::Vec3> const& center) const
{
    if (!m_config)
    {
        return;
    }

    for (const auto& nodeWithTransformsPair : m_nodesWithTransforms)
    {
        if (auto cluster = getOrCreateCluster(nodeWithTransformsPair.first,
                RailwayMeshGeneratorConfig::MaterialInfo{}))
        {
            addTransformsToCluster(cluster, center, nodeWithTransformsPair.second);
        }
    }
}

bool
DynamicRailwayAdjunct::isLeftSide(const RailwayMeshGeneratorConfig::AdjunctPartData& part) const
{
    return part.baselineOffsetSideMm < 0;
}

Math::Vec3
DynamicRailwayAdjunct::getOffsetVector(
    const RailwayMeshGeneratorConfig::AdjunctPartData& part) const
{
    double offsetFromRails = RailwayMeshGeneratorConfig::toInnerUnits(part.baselineOffsetSideMm);
    double height = RailwayMeshGeneratorConfig::toInnerUnits(part.baselineOffsetHeightMm);

    return Math::Vec3(0.0, offsetFromRails, height);
}
