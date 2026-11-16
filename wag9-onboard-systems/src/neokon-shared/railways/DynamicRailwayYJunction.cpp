#include "DynamicRailwayYJunction.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/NormalizedAngleFromTo.h"
#include "neokon-shared/railways/RailSwitchInfoComponents.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConstants.h"
#include "neokon-shared/TransformFromPositionAndAxis.h"

#include <random>

using namespace Unigine;

namespace
{

const int leftPathIdx = 0;
const int rightPathIdx = 1;
const int totalPaths = 2;

}// anonymous namespace

DynamicRailwayYJunction::DynamicRailwayYJunction(uint32_t partId, const std::string& configPath,
    std::vector<BezierSpline>&& splines,
    std::vector<SplineNeighborConfigs>&& neighborConfigs,
    const Math::Mat4& transform)
    : DynamicRailwayJunctionBase(partId, configPath, std::move(splines),
        std::move(neighborConfigs), transform)
{
}

bool DynamicRailwayYJunction::load()
{
    if (!loadConfig())
    {
        return false;
    }

    setMetaObject(MetaUnigine::ObjectMeshStatic::create());
    getMetaObject()->setWorldTransform(getTransform());

    auto mesh = Mesh::create();

    SharedDataArgs sharedData{ mesh };
    if (!buildSplines(sharedData))
    {
        return false;
    }

    generateEmbankment(sharedData);

    if (isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::JunctionRails))
    {
        generateRailFrog(sharedData, +InternalSplineType::PathRight_LeftRail,
            +InternalSplineType::PathLeft_RightRail, /*flip=*/true);
        if (!generatePointRails(sharedData))
        {
            return false;
        }
        generateRails(sharedData);
        if (!sharedData.railFrogsInfo.empty())
        {
            generateGuardRailsForRailFrog(sharedData, *sharedData.railFrogsInfo.front(),
                sharedData.guardRailClosureOffset);
        }
    }

    sharedData.sleepersInfos.emplace_back(new DynamicRailwayPartWithSleepers::Sleepers);
    sharedData.sleepersInfos.emplace_back(new DynamicRailwayPartWithSleepers::Sleepers);

    auto addMiddleSleeperInfoFunc = [this, &sharedData](const Math::Mat4& middle)
    {
        auto worldTransform = getTransform() * middle;

        for (int pathIdx = 0; pathIdx < sharedData.sleepersInfos.size(); ++pathIdx)
        {
            sharedData.sleepersInfos[pathIdx]->emplace_back(
                DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
        }
    };

    generateMiddleSleepers(sharedData, sharedData.middleSpline,
        interFillVec(+InternalSplineType::PathLeft_LeftRail, +InternalSplineType::PathRight_RightRail),
        [this, &sharedData, &addMiddleSleeperInfoFunc](double startDst, double endDst, double step,
            int type, bool skipFirst, bool skipLast, const AddMountsFuncs& addMountsFuncs)
    {
        doAddMiddleSleepersSwitchBoxOverride(sharedData, sharedData.middleSpline, startDst, endDst,
            step, type, skipFirst, skipLast, sharedData.leftOuterSplinesIds,
            sharedData.rightOuterSplinesIds,
            +RailwayMeshGeneratorConfig::SwitchRodsType::Default, /*invertBoxSide=*/false,
            addMountsFuncs, addMiddleSleeperInfoFunc);
    });

    generateSleepersOnOuts(sharedData);
    setSleepers(std::move(sharedData.sleepersInfos));

    mesh->createBounds();
    getMetaObject()->setMesh(mesh);

    addJunctionProperty(sharedData);
    addConnectionPoints(sharedData);

    setupMeshMaterials(sharedData);

    return true;
}

bool DynamicRailwayYJunction::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto segment = std::dynamic_pointer_cast<DynamicRailwayYJunction>(old);
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

NodePtr
DynamicRailwayYJunction::createNodes()
{
    auto result = DynamicRailwayJunctionBase::createNodes();
    if (result && !isSwitchedToRight())
    {
        // Model is generated switched to right, so switch it here if left position was requested
        // before generation
        railSwitchToLeft(result);
    }
    return result;
}

void
DynamicRailwayYJunction::switchToRight()
{
    if (isSwitchedToRight())
    {
        return;
    }

    if (auto model = getCreatedNodes())
    {
        railSwitchToLeft(model);
    }

    m_switchState = SwitchState::Right;
}

void
DynamicRailwayYJunction::switchToLeft()
{
    if (!isSwitchedToRight())
    {
        return;
    }

    if (auto model = getCreatedNodes())
    {
        railSwitchToRight(model);
    }

    m_switchState = SwitchState::Left;
}

bool
DynamicRailwayYJunction::isSwitchedToRight() const
{
    return m_switchState == SwitchState::Right;
}

void DynamicRailwayYJunction::generateLODs(SharedDataArgs& sharedData,
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

//------------------------------------------------------------------------------------------
// Generation steps
//------------------------------------------------------------------------------------------
bool DynamicRailwayYJunction::buildSplines(SharedDataArgs& sharedData)
{
    message("building temporary splines...\n");

    auto const& railProfileLOD0 = railFacePositionsLOD0();
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

    const double trackWidth = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getCommonData().railwayTrackWidth);
    const double profileWidth = maxProfileY - minProfileY;
    const double halfProfileWidth = profileWidth * 0.5;
    const double offset = (trackWidth + halfProfileWidth) * 0.5;
    const double guardRailStockOffset = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getGuardRailsData().offset);

    sharedData.railProfileWidth = profileWidth;
    sharedData.guardRailClosureOffset = offset * 2. - guardRailStockOffset;

    assert(getSplines().size() == 2);

    auto leftStart = getSplines()[leftPathIdx].pointAtDistance(0.0);
    auto leftEnd = getSplines()[leftPathIdx].pointAtDistance(getSplines()[leftPathIdx].arcLength());
    if (!leftStart || !leftEnd)
    {
        error("Unable to find left spline edges\n");
        return false;
    }
    auto leftVec = *leftEnd - *leftStart;

    auto rightStart = getSplines()[rightPathIdx].pointAtDistance(0.0);
    auto rightEnd =
        getSplines()[rightPathIdx].pointAtDistance(getSplines()[rightPathIdx].arcLength());
    if (!rightStart || !rightEnd)
    {
        error("Unable to find right spline edges\n");
        return false;
    }
    auto rightVec = *rightEnd - *rightStart;

    if (glm::dot(rightVec, leftVec) < 0.0)
    {
        error("Opposite paths are not supported\n");
        return false;
    }

    auto up = glm::cross(rightVec, leftVec);
    if (up.z < 0.0)
    {
        warning("The right path is mixed up with the left path, fixing an input data.\n");
        swapSplines(leftPathIdx, rightPathIdx);
    }

    sharedData.splineSearchById[+InternalSplineType::PathLeft].build(getSplines()[leftPathIdx]);
    sharedData.splineSearchById[+InternalSplineType::PathRight].build(getSplines()[rightPathIdx]);
    sharedData.middleSpline.build(getSplines()[leftPathIdx], getSplines()[rightPathIdx],
        [](double) { return 0.5; });

    addRail(sharedData, leftPathIdx, +InternalSplineType::PathLeft_LeftRail, "PathLeft_LeftRail",
        offset, /*mirror=*/false);
    addRail(sharedData, leftPathIdx, +InternalSplineType::PathLeft_RightRail, "PathLeft_RightRail",
        -offset, /*mirror=*/true);

    addRail(sharedData, rightPathIdx, +InternalSplineType::PathRight_LeftRail, "PathRight_LeftRail",
        offset, /*mirror=*/false);
    addRail(sharedData, rightPathIdx, +InternalSplineType::PathRight_RightRail,
        "PathRight_RightRail", -offset, /*mirror=*/true);

    auto crossPoint = sharedData.splineSearchById[+InternalSplineType::PathLeft_RightRail].
        findPointsWithDistance(sharedData.splineById[+InternalSplineType::PathRight_LeftRail], 0.0);
    if (!crossPoint)
    {
        error("Unable to find closure cross point\n");
        return false;
    }
    if (crossPoint->gap > 0.1)
    {
        warning("crossPoint->gap == %d > 0.1, bad y-junction\n", crossPoint->gap);
    }

    sharedData.closureCrossPoint = *crossPoint;

    auto closureCrossPointPosition = sharedData.closureCrossPoint.first.transform.getTranslate();
    auto middleCrossPoint =
        sharedData.middleSpline.findClosestPoint(toGlm(closureCrossPointPosition));
    if (!middleCrossPoint)
    {
        error("Unable to find middle cross point\n");
        return false;
    }
    double middleCrossError = (middleCrossPoint->transform.getTranslate() -
        closureCrossPointPosition).length();
    if (middleCrossError > 0.2)
    {
        warning("Too big distance between sleepers and closure spline's cross points == %f\n",
            middleCrossError);
    }
    sharedData.middleCrossPoint = *middleCrossPoint;

    sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::PathLeft_LeftRail);
    sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::PathRight_RightRail);

    sharedData.oppositeSplineIds = {
        {+InternalSplineType::PathLeft_LeftRail, +InternalSplineType::PathLeft_RightRail},
        {+InternalSplineType::PathLeft_RightRail, +InternalSplineType::PathLeft_LeftRail},
        {+InternalSplineType::PathRight_LeftRail, +InternalSplineType::PathRight_RightRail},
        {+InternalSplineType::PathRight_RightRail, +InternalSplineType::PathRight_LeftRail}
    };

    return true;
}

bool DynamicRailwayYJunction::generatePointRails(SharedDataArgs& sharedData)
{
    message("generating point rails...\n");

    const double pointRailStockOffset = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getPointRailsData().stockOffset);

    const auto& leftJoinSplineSearch =
        getSplineSearch(sharedData, +InternalSplineType::PathLeft_LeftRail);
    const auto& leftPointSpline = getSpline(sharedData, +InternalSplineType::PathRight_LeftRail);

    auto leftPointRailMountPoint = leftJoinSplineSearch.findPointsWithDistance(leftPointSpline,
            pointRailStockOffset + sharedData.railProfileWidth);
    if (!leftPointRailMountPoint)
    {
        error("Unable to find left point rail mount point\n");
        return false;
    }
    auto leftStartTransform = leftPointSpline.calcSplineMatrixAtDistance(0.0);
    if (!leftStartTransform)
    {
        error("Unable to calc left start transform\n");
        return false;
    }
    TransformSpline::Point leftPointRailStart{0.0, *leftStartTransform};
    const auto& leftPointRailEnd = leftPointRailMountPoint->second;
    auto leftPointRail = generatePointRail(sharedData, +InternalSplineType::PathRight_LeftRail,
        +InternalSplineType::PathLeft_LeftRail, leftPointRailStart, leftPointRailEnd, true, true,
        "LeftArrowRail_Anim");
    generateHeelBlock(sharedData, +InternalSplineType::PathRight_LeftRail,
        +InternalSplineType::PathLeft_LeftRail, leftPointRailEnd, /*leftSide=*/true);

    const auto& rightJoinSpline = getSpline(sharedData, +InternalSplineType::PathRight_RightRail);
    const auto& rightPointSpline = getSpline(sharedData, +InternalSplineType::PathLeft_RightRail);

    auto rightStartTransform = rightPointSpline.calcSplineMatrixAtDistance(0.0);
    auto rightEndTransform =
        rightPointSpline.calcSplineMatrixAtDistance(leftPointRailMountPoint->second.distanceOffset);
    if (!rightStartTransform  || !rightEndTransform)
    {
        error("Unable to calc right start/end transforms\n");
        return false;
    }
    TransformSpline::Point rightPointRailEnd = {leftPointRailMountPoint->second.distanceOffset,
        *rightEndTransform};
    TransformSpline::Point rightPointRailStart{0.0, *rightStartTransform};
    auto rightPointRail = generatePointRail(sharedData, +InternalSplineType::PathLeft_RightRail,
        +InternalSplineType::PathRight_RightRail, rightPointRailStart, rightPointRailEnd,
        /*isLeft=*/false,
        /*isFront=*/true,
        "RightArrowRail_Anim");
    generateHeelBlock(sharedData, +InternalSplineType::PathLeft_RightRail,
        +InternalSplineType::PathRight_RightRail, rightPointRailEnd, /*leftSide=*/false);

    if (!leftPointRail || !rightPointRail)
    {
        return false;
    }

    generateSwitchRods(sharedData, *leftPointRail, *rightPointRail, "_Anim",
        /*initialRightTransform=*/false, RailwayMeshGeneratorConfig::SwitchRodsType::Default);

    return true;
}

void DynamicRailwayYJunction::generateSleepersOnOuts(SharedDataArgs& sharedData)
{
    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);
    if (sharedData.firstSleeperTransform)
    {
        auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
        {
            auto worldTransform = getTransform() * transform;

            for (int pathIdx = 0; pathIdx < sharedData.sleepersInfos.size(); ++pathIdx)
            {
                sharedData.sleepersInfos[pathIdx]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            }
        };

        generateSleepersOnOut(sharedData, +InternalSplineType::PathLeft,
            sharedData.firstSleeperTransform->transform, sleeperStep / 2., addSleeperInfoFunc);
    }
    else
    {
        Log::error("sharedData.firstSleeperTransform == std::nullopt for %d junction\n", m_key.id);
    }

    if (sharedData.lastSleeperTransform)
    {
        auto addSleeperInfoFunc0 = [this, &sharedData](const Math::Mat4& transform)
        {
            auto worldTransform = getTransform() * transform;

            sharedData.sleepersInfos[+InternalSplineType::PathLeft]->emplace_back(
                DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
        };
        generateSleepersOnOut(sharedData, +InternalSplineType::PathLeft,
            sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc0);

        auto addSleeperInfoFunc1 = [this, &sharedData](const Math::Mat4& transform)
        {
            auto worldTransform = getTransform() * transform;

            sharedData.sleepersInfos[+InternalSplineType::PathRight]->emplace_back(
                DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
        };
        generateSleepersOnOut(sharedData, +InternalSplineType::PathRight,
            sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc1);
    }
    else
    {
        Log::error("sharedData.lastSleeperTransform == std::nullopt for %d junction\n", m_key.id);
    }
}

void DynamicRailwayYJunction::addJunctionProperty(SharedDataArgs& sharedData)
{
    message("adding juncton property component...\n");

    int propIndex = getMetaObject()->findProperty("junction");
    if (propIndex < 0)
    {
        propIndex = getMetaObject()->addProperty("junction");

        PropertyPtr props = getMetaObject()->getProperty(propIndex);
        PropertyParameterPtr typeParam = props->getParameterPtr("type");
        typeParam->setValueSwitch(JUNCTION_TYPE_Y);
    }
}

void DynamicRailwayYJunction::addConnectionPoints(SharedDataArgs& sharedData)
{
    message("adding connection points...\n");

    const int JUNCTION_POINT_CENTRAL = 0;
    const int JUNCTION_POINT_INPUT = 1;
    const int JUNCTION_POINT_LEFT = 2;
    const int JUNCTION_POINT_RIGHT = 3;
    const int JUNCTION_POINT_TOTAL = 4;

    Math::Vec3 points[JUNCTION_POINT_TOTAL];

    for (int pathIdx = 0; pathIdx < totalPaths; ++pathIdx)
    {
        if (pathIdx == leftPathIdx)
        {
            const auto& segments = getSplines()[leftPathIdx].getSegmentsMap();
            points[JUNCTION_POINT_INPUT] = fromGlm(segments(0)->p0());
            points[JUNCTION_POINT_LEFT] = fromGlm(segments(segments.iterative_size() - 1)->p3());
        }
        else if (pathIdx == rightPathIdx)
        {
            const auto& segments = getSplines()[rightPathIdx].getSegmentsMap();
            points[JUNCTION_POINT_CENTRAL] = fromGlm(segments(0)->p0());
            points[JUNCTION_POINT_RIGHT] = fromGlm(segments(segments.iterative_size() - 1)->p3());
        }
    }

    for (int i = 0; i < JUNCTION_POINT_TOTAL; i++)
    {
        auto node = MetaUnigine::NodeDummy::create();
        getMetaObject()->addChild(node);

        node->setPosition(points[i]);
        int propIdx = node->addProperty("cross_helper");
        if (propIdx >= 0)
        {
            Unigine::PropertyPtr props = node->getProperty(propIdx);
            Unigine::PropertyParameterPtr indexParam = props->getParameterPtr("index");
            indexParam->setValueInt(i - 1);
        }
    }
}
