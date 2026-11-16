#include "DynamicRailwayThreeWayJunction.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/NormalizedAngleFromTo.h"
#include "neokon-shared/StringFormat.h"
#include "neokon-shared/TransformFromPositionAndAxis.h"
#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConstants.h"
#include "neokon-shared/railways/RailSwitchInfoComponents.h"

using namespace Unigine;

namespace
{

const String LeftPointRailNodeName = "LeftPointRail";
const String RightPointRailNodeName = "RightPointRail";

const double PointRailMinFrogOffsetM = 0.6;

} // Anonymous namespace

const char* DynamicRailwayThreeWayJunction::NearSwitchSuffix = "ThreeWay_AnimNear";
const char* DynamicRailwayThreeWayJunction::MiddleSwitchSuffix = "ThreeWay_AnimMiddle";

DynamicRailwayThreeWayJunction::DynamicRailwayThreeWayJunction(uint32_t partId,
    const std::string& configPath, std::vector<BezierSpline>&& splines,
    std::vector<SplineNeighborConfigs>&& neighborConfigs, const Unigine::Math::Mat4& transform)
    : DynamicRailwayJunctionBase(partId, configPath, std::move(splines), std::move(neighborConfigs),
        transform)
{
}

bool
DynamicRailwayThreeWayJunction::load()
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
        auto middleFrog = generateRailFrog(sharedData, +InternalSplineType::RailToRightLeft,
            +InternalSplineType::RailToLeftRight, /*flip=*/true, /*isObtuse=*/false);
        if (middleFrog)
        {
            generateGuardRailsForRailFrog(sharedData, *middleFrog,
                sharedData.guardRailClosureOffset);
        }

        if (sharedData.isLeftSide)
        {
            auto firstSideFrog = generateRailFrog(sharedData, +InternalSplineType::RailToRightLeft,
                +InternalSplineType::RailToSideRight, /*flip=*/true, /*isObtuse=*/false);
            if (firstSideFrog)
            {
                generateMiddlePointRailsForLeftSide(sharedData, *firstSideFrog);
                generateRootPointRailsForLeftSide(sharedData, *firstSideFrog);
                generateGuardRailsForRailFrog(sharedData, *firstSideFrog,
                    sharedData.guardRailClosureOffset);
            }

            auto secondSideFrog = generateRailFrog(sharedData, +InternalSplineType::RailToLeftLeft,
                +InternalSplineType::RailToSideRight, /*flip=*/true, /*isObtuse=*/false);
            if (secondSideFrog)
            {
                generateGuardRailsForRailFrog(sharedData, *secondSideFrog,
                    sharedData.guardRailClosureOffset);
            }

            m_isLeftSide = true;
        }
        else if (sharedData.isRightSide)
        {
            auto firstSideFrog = generateRailFrog(sharedData, +InternalSplineType::RailToSideLeft,
                +InternalSplineType::RailToLeftRight, /*flip=*/true, /*isObtuse=*/false);
            if (firstSideFrog)
            {
                generateMiddlePointRailsForRightSide(sharedData, *firstSideFrog);
                generateRootPointRailsForRightSide(sharedData, *firstSideFrog);
                generateGuardRailsForRailFrog(sharedData, *firstSideFrog,
                    sharedData.guardRailClosureOffset);
            }

            auto secondSideFrog = generateRailFrog(sharedData, +InternalSplineType::RailToSideLeft,
                +InternalSplineType::RailToRightRight, /*flip=*/true, /*isObtuse=*/false);
            if (secondSideFrog)
            {
                generateGuardRailsForRailFrog(sharedData, *secondSideFrog,
                    sharedData.guardRailClosureOffset);
            }
        }

        generateRails(sharedData);
    }

    for (int pathIdx = 0; pathIdx < 4; ++pathIdx)
    {
        sharedData.sleepersInfos.emplace_back(new DynamicRailwayPartWithSleepers::Sleepers);
    }

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
        interFillVec(+InternalSplineType::RailToSideLeft,
        +InternalSplineType::RailToRightRight),
        addMiddleSleeperInfoFunc);

    generateSleepersOnOuts(sharedData);
    setSleepers(std::move(sharedData.sleepersInfos));

    mesh->createBounds();
    getMetaObject()->setMesh(mesh);

    addJunctionProperty(sharedData);
    addConnectionPoints(sharedData);
    setupMeshMaterials(sharedData);

    return true;
}

bool
DynamicRailwayThreeWayJunction::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto part = std::dynamic_pointer_cast<DynamicRailwayThreeWayJunction>(old);
    if (!part)
    {
        return true;
    }
    if (part.get() != this)
    {
        return true;
    }
    return DynamicRailwayPartGenerated::isDifferentWith(old);
}

NodePtr
DynamicRailwayThreeWayJunction::createNodes()
{
    auto result = DynamicRailwayJunctionBase::createNodes();
    if (result)
    {
        // Model is generated switched right to right, so switch it here if other was requested
        // before generation
        SwitchState rootSwitchState = m_rootSwitchState;
        SwitchState middleSwitchState = m_middleSwitchState;
        m_rootSwitchState = m_middleSwitchState = SwitchState::Right;
        doSwitch(rootSwitchState, middleSwitchState);
    }
    return result;
}

void
DynamicRailwayThreeWayJunction::switchToSide()
{
    doSwitch(isLeftSide() ? SwitchState::Left : SwitchState::Right, SwitchState::Left);
}

void
DynamicRailwayThreeWayJunction::switchToLeft()
{
    doSwitch(isLeftSide() ? SwitchState::Right : SwitchState::Left, SwitchState::Left);
}

void
DynamicRailwayThreeWayJunction::switchToRight()
{
    doSwitch(isLeftSide() ? SwitchState::Right : SwitchState::Left, SwitchState::Right);
}

void
DynamicRailwayThreeWayJunction::doSwitch(SwitchState root, SwitchState middle)
{
    if (m_rootSwitchState != root)
    {
        if (auto model = getCreatedNodes())
        {
            if (root == SwitchState::Right)
            {
                railSwitchToLeft(model, NearSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, NearSwitchSuffix);
            }
        }
        m_rootSwitchState = root;
    }

    if (m_middleSwitchState != middle)
    {
        if (auto model = getCreatedNodes())
        {
            if (middle == SwitchState::Right)
            {
                railSwitchToLeft(model, MiddleSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, MiddleSwitchSuffix);
            }
        }
        m_middleSwitchState = middle;
    }
}

//------------------------------------------------------------------------------------------
// Generation steps
//------------------------------------------------------------------------------------------
void
DynamicRailwayThreeWayJunction::validateMiddleSplines(SharedDataArgs& sharedData)
{
    const auto& leftSpline = getSpline(sharedData, +InternalSplineType::PathMiddleToLeft);
    auto leftStart = leftSpline.pointAtDistance(0.0);
    auto leftEnd = leftSpline.pointAtDistance(leftSpline.arcLength());
    if (!leftStart || !leftEnd)
    {
        error("Unable to find left spline edges\n");
        return;
    }
    auto leftVec = *leftEnd - *leftStart;

    const auto& rightSpline = getSpline(sharedData, +InternalSplineType::PathMiddleToRight);
    auto rightStart = rightSpline.pointAtDistance(0.0);
    auto rightEnd = rightSpline.pointAtDistance(rightSpline.arcLength());
    if (!rightStart || !rightEnd)
    {
        error("Unable to find right spline edges\n");
        return;
    }
    auto rightVec = *rightEnd - *rightStart;

    if (glm::dot(rightVec, leftVec) < 0.0)
    {
        error("Opposite paths are not supported\n");
        return;
    }

    auto up = glm::cross(rightVec, leftVec);
    if (up.z < 0.0)
    {
        warning("The right path is mixed up with the left path, fixing an input data.\n");
        swapSplines(+InternalSplineType::PathMiddleToLeft, +InternalSplineType::PathMiddleToRight);
    }
}

bool
DynamicRailwayThreeWayJunction::buildSplines(SharedDataArgs& sharedData)
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

    const double trackWidth =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getCommonData().railwayTrackWidth);
    const double profileWidth = maxProfileY - minProfileY;
    const double halfProfileWidth = profileWidth * 0.5;
    const double railOffset = (trackWidth + halfProfileWidth) * 0.5;
    const double sleeperWidth =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().minLength);
    const double halfSleeperWidth = sleeperWidth * 0.5;
    const double guardRailStockOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getGuardRailsData().offset);

    sharedData.railOffset = railOffset;
    sharedData.railProfileWidth = profileWidth;
    sharedData.guardRailClosureOffset = railOffset * 2. - guardRailStockOffset;
    sharedData.pointRailStockOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getPointRailsData().stockOffset) +
        profileWidth;

    validateMiddleSplines(sharedData);

    // paths for sleepers on outs
    sharedData.splineSearchById[+InternalSplineType::PathRootToSide].build(
        getSpline(sharedData, +InternalSplineType::PathRootToSide));
    sharedData.splineSearchById[+InternalSplineType::PathMiddleToLeft].build(
        getSpline(sharedData, +InternalSplineType::PathMiddleToLeft));
    sharedData.splineSearchById[+InternalSplineType::PathMiddleToRight].build(
        getSpline(sharedData, +InternalSplineType::PathMiddleToRight));

    // To side
    addRail(sharedData, +InternalSplineType::PathRootToSide, +InternalSplineType::RailToSideLeft,
        "RailToSideLeft", railOffset, /*mirror=*/false);
    addRail(sharedData, +InternalSplineType::PathRootToSide, +InternalSplineType::RailToSideRight,
        "RailToSideRight", -railOffset, /*mirror=*/true);

    // To middle
    addRail(sharedData, +InternalSplineType::PathRootToMiddle, +InternalSplineType::RailToMiddleLeft,
        "RailToMiddleLeft", railOffset, /*mirror=*/false);
    addRail(sharedData, +InternalSplineType::PathRootToMiddle, +InternalSplineType::RailToMiddleRight,
        "RailToMiddleRight", -railOffset, /*mirror=*/true);

    // To left
    addRail(sharedData, +InternalSplineType::PathMiddleToLeft, +InternalSplineType::RailToLeftLeft,
        "RailToLeftLeft", railOffset, /*mirror=*/false);
    addRail(sharedData, +InternalSplineType::PathMiddleToLeft, +InternalSplineType::RailToLeftRight,
        "RailToLeftRight", -railOffset, /*mirror=*/true);

    // To right
    addRail(sharedData, +InternalSplineType::PathMiddleToRight, +InternalSplineType::RailToRightLeft,
        "RailToRightLeft", railOffset, /*mirror=*/false);
    addRail(sharedData, +InternalSplineType::PathMiddleToRight, +InternalSplineType::RailToRightRight,
        "RailToRightRight", -railOffset, /*mirror=*/true);

    if (sharedData.isLeftSide = isLeftSideJunction(sharedData))
    {
        BezierSpline mergedRight;
        mergedRight.appendSpline(getSpline(sharedData, +InternalSplineType::PathRootToMiddle));
        mergedRight.appendSpline(getSpline(sharedData, +InternalSplineType::PathMiddleToRight));
        sharedData.middleSpline.build(getSpline(sharedData, +InternalSplineType::PathRootToSide),
            mergedRight, [](double) { return 0.5; });
        sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::RailToSideLeft);
        sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::RailToMiddleRight);
        sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::RailToRightRight);
    }
    else if (sharedData.isRightSide = isRightSideJunction(sharedData))
    {
        BezierSpline mergedLeft;
        mergedLeft.appendSpline(getSpline(sharedData, +InternalSplineType::PathRootToMiddle));
        mergedLeft.appendSpline(getSpline(sharedData, +InternalSplineType::PathMiddleToLeft));
        sharedData.middleSpline.build(mergedLeft,
            getSpline(sharedData, +InternalSplineType::PathRootToSide), [](double) { return 0.5; });
        sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::RailToMiddleLeft);
        sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::RailToLeftLeft);
        sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::RailToSideRight);
    }

    sharedData.oppositeSplineIds = {
        {+InternalSplineType::RailToSideLeft, +InternalSplineType::RailToSideRight},
        {+InternalSplineType::RailToSideRight, +InternalSplineType::RailToSideLeft},
        {+InternalSplineType::RailToMiddleLeft, +InternalSplineType::RailToMiddleRight},
        {+InternalSplineType::RailToMiddleRight, +InternalSplineType::RailToMiddleLeft},
        {+InternalSplineType::RailToLeftLeft, +InternalSplineType::RailToLeftRight},
        {+InternalSplineType::RailToLeftRight, +InternalSplineType::RailToLeftLeft},
        {+InternalSplineType::RailToRightLeft, +InternalSplineType::RailToRightRight},
        {+InternalSplineType::RailToRightRight, +InternalSplineType::RailToRightLeft}
    };

    return true;
}

std::optional<TransformSpline::NeighbourPoints>
DynamicRailwayThreeWayJunction::findPointRailRoot(SharedDataArgs& sharedData, double offset,
    int leftSplineIdx, int rightSplineIdx, double startM, double endM)
{
    const auto& leftSearch = getSplineSearch(sharedData, leftSplineIdx);
    const auto& rightSpline = getSpline(sharedData, rightSplineIdx);

    return leftSearch.findPointsWithDistance(rightSpline, offset, 0.1, startM, endM);
}

void
DynamicRailwayThreeWayJunction::generateMiddlePointRailsForLeftSide(SharedDataArgs& sharedData,
    const RailFrogInfo& firstSideFrog)
{
    double startDistance = 0.0f;
    double endDistance = firstSideFrog.crossPoint.first.distanceOffset;
    if (firstSideFrog.leftInputSplineInterval)
    {
        endDistance = firstSideFrog.leftInputSplineInterval->first;
    }
    endDistance -= PointRailMinFrogOffsetM;

    auto leftEndPoints = findPointRailRoot(sharedData, sharedData.pointRailStockOffset,
        +InternalSplineType::RailToLeftLeft, +InternalSplineType::RailToRightLeft,
        startDistance, endDistance);
    if (!leftEndPoints)
    {
        return;
    }
    endDistance = leftEndPoints->second.distanceOffset;

    auto rightStartPoints = findPointRailRoot(sharedData, sharedData.pointRailStockOffset,
        +InternalSplineType::RailToSideRight,
        +InternalSplineType::RailToLeftRight, startDistance, endDistance);
    if (rightStartPoints && (endDistance > rightStartPoints->second.distanceOffset))
    {
        startDistance = rightStartPoints->second.distanceOffset;
    }

    // Far left

    const TransformSpline::Point& leftEndPoint = leftEndPoints->second;
    const auto& railToRightLeft = getSpline(sharedData, +InternalSplineType::RailToRightLeft);
    const auto leftStartTransform = railToRightLeft.calcSplineMatrixAtDistance(startDistance);
    if (!leftStartTransform)
    {
        return;
    }

    TransformSpline::Point leftStartPoint{startDistance, *leftStartTransform};
    auto leftPointRail = generatePointRail(sharedData, +InternalSplineType::RailToRightLeft,
        +InternalSplineType::RailToLeftLeft, leftStartPoint, leftEndPoint, /*isLeftType=*/true,
        /*isFront=*/true, LeftPointRailNodeName + MiddleSwitchSuffix);
    if (leftPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToRightLeft,
            +InternalSplineType::RailToLeftLeft, leftEndPoint, true);
    }

    // Far right

    const auto& railToLeftRight = getSpline(sharedData, +InternalSplineType::RailToLeftRight);
    const auto rightStartTransform = railToLeftRight.calcSplineMatrixAtDistance(startDistance);
    const auto rightEndTransform = railToLeftRight.calcSplineMatrixAtDistance(endDistance);
    if (!rightStartTransform || !rightEndTransform)
    {
        return;
    }

    TransformSpline::Point rightStartPoint{startDistance, *rightStartTransform};
    TransformSpline::Point rightEndPoint{endDistance, *rightEndTransform};

    auto rightPointRail = generatePointRail(sharedData, +InternalSplineType::RailToLeftRight,
        +InternalSplineType::RailToRightRight, rightStartPoint, rightEndPoint, /*isLeftType=*/false,
        /*isFront=*/true, RightPointRailNodeName + MiddleSwitchSuffix);
    if (rightPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToLeftRight,
            +InternalSplineType::RailToRightRight, rightEndPoint, false);
    }

    if (leftPointRail && rightPointRail)
    {
        generateSwitchRods(sharedData, *leftPointRail, *rightPointRail, MiddleSwitchSuffix,
            RailwayMeshGeneratorConfig::SwitchRodsType::TWSecond);
        generateCommonSwitchBox(Math::lerp(*leftStartTransform, *rightStartTransform, 0.5),
            RailwayMeshGeneratorConfig::SwitchRodsType::TWSecond);
    }
}

void
DynamicRailwayThreeWayJunction::generateRootPointRailsForLeftSide(SharedDataArgs& sharedData,
    const RailFrogInfo& firstSideFrog)
{
    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);
    
    double startDistance = sleeperStep / 2.;

    // Near left

    const auto& railToMiddleLeft = getSpline(sharedData, +InternalSplineType::RailToMiddleLeft);
    double endDistance = railToMiddleLeft.arcLength() - sleeperStep;

    auto leftEndPoints = findPointRailRoot(sharedData, sharedData.pointRailStockOffset,
        +InternalSplineType::RailToSideLeft, +InternalSplineType::RailToMiddleLeft, startDistance,
        endDistance);
    if (!leftEndPoints)
    {
        return;
    }
    const TransformSpline::Point& leftEndPoint = leftEndPoints->second;

    const auto leftStartTransform = railToMiddleLeft.calcSplineMatrixAtDistance(startDistance);
    if (!leftStartTransform)
    {
        return;
    }
    TransformSpline::Point leftStartPoint{startDistance, *leftStartTransform};
    auto leftPointRail = generatePointRail(sharedData, +InternalSplineType::RailToMiddleLeft,
        +InternalSplineType::RailToSideLeft, leftStartPoint, leftEndPoint, /*isLeftType=*/true,
        /*isFront=*/true, LeftPointRailNodeName + NearSwitchSuffix);
    if (leftPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToMiddleLeft,
            +InternalSplineType::RailToSideLeft, leftEndPoint, true);
    }

    // Near right

    Math::dvec3 from = leftEndPoint.transform.getTranslate();
    Math::dvec3 dir = leftEndPoint.transform.getAxisY();

    auto rightEndPoints = getSplineSearch(sharedData, +InternalSplineType::RailToSideRight)
                                  .findClosestPoints(toGlm(from - dir * sharedData.railOffset),
                                      toGlm(from - dir * sharedData.railOffset * 2.5));
    if (!rightEndPoints)
    {
        return;
    }
    const TransformSpline::Point& rightEndPoint = rightEndPoints->first;

    const auto& railToSideRight = getSpline(sharedData, +InternalSplineType::RailToSideRight);
    const auto rightStartTransform = railToSideRight.calcSplineMatrixAtDistance(startDistance);
    if (!rightStartTransform)
    {
        return;
    }

    TransformSpline::Point rightStartPoint{startDistance, *rightStartTransform};
    auto rightPointRail = generatePointRail(sharedData, +InternalSplineType::RailToSideRight,
        +InternalSplineType::RailToMiddleRight, rightStartPoint, rightEndPoint,
        /*isLeftType=*/false, /*isFront=*/true, RightPointRailNodeName + NearSwitchSuffix);
    if (rightPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToSideRight,
            +InternalSplineType::RailToMiddleRight, rightEndPoint, false);
    }

    if (leftPointRail && rightPointRail)
    {
        generateSwitchRods(sharedData, *leftPointRail, *rightPointRail, NearSwitchSuffix,
            RailwayMeshGeneratorConfig::SwitchRodsType::TWFirst);
        generateCommonSwitchBox(Math::lerp(*leftStartTransform, *rightStartTransform, 0.5),
            RailwayMeshGeneratorConfig::SwitchRodsType::TWFirst);
    }
}

void
DynamicRailwayThreeWayJunction::generateMiddlePointRailsForRightSide(SharedDataArgs& sharedData,
    const RailFrogInfo& firstSideFrog)
{
    double startDistance = 0.0f;
    double endDistance = firstSideFrog.crossPoint.second.distanceOffset;
    if (firstSideFrog.rightInputSplineInterval)
    {
        endDistance = firstSideFrog.rightInputSplineInterval->first;
    }
    endDistance -= PointRailMinFrogOffsetM;

    auto rightEndPoints = findPointRailRoot(sharedData, sharedData.pointRailStockOffset,
        +InternalSplineType::RailToLeftRight, +InternalSplineType::RailToRightRight, startDistance,
        endDistance);
    if (!rightEndPoints)
    {
        return;
    }
    endDistance = rightEndPoints->first.distanceOffset;

    auto leftStartPoints = findPointRailRoot(sharedData, sharedData.pointRailStockOffset,
        +InternalSplineType::RailToSideLeft, +InternalSplineType::RailToRightLeft, startDistance,
        endDistance);
    if (leftStartPoints && (endDistance > leftStartPoints->second.distanceOffset))
    {
        startDistance = leftStartPoints->second.distanceOffset;
    }

    // Far right

    const TransformSpline::Point& rightEndPoint = rightEndPoints->first;
    const auto& railToLeftRight = getSpline(sharedData, +InternalSplineType::RailToLeftRight);
    const auto rightStartTransform = railToLeftRight.calcSplineMatrixAtDistance(startDistance);
    if (!rightStartTransform)
    {
        return;
    }

    TransformSpline::Point rightStartPoint{startDistance, *rightStartTransform};
    auto rightPointRail = generatePointRail(sharedData, +InternalSplineType::RailToLeftRight,
        +InternalSplineType::RailToRightRight, rightStartPoint, rightEndPoint, /*isLeftType=*/false,
        /*isFront=*/true, RightPointRailNodeName + MiddleSwitchSuffix);
    if (rightPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToLeftRight,
            +InternalSplineType::RailToRightRight, rightEndPoint, true);
    }

    // Far left

    const auto& railToRightLeft = getSpline(sharedData, +InternalSplineType::RailToRightLeft);
    const auto leftStartTransform = railToRightLeft.calcSplineMatrixAtDistance(startDistance);
    const auto leftEndTransform = railToRightLeft.calcSplineMatrixAtDistance(endDistance);
    if (!leftStartTransform || !leftEndTransform)
    {
        return;
    }

    TransformSpline::Point leftStartPoint{startDistance, *leftStartTransform};
    TransformSpline::Point leftEndPoint{endDistance, *leftEndTransform};

    auto leftPointRail = generatePointRail(sharedData, +InternalSplineType::RailToRightLeft,
        +InternalSplineType::RailToLeftLeft, leftStartPoint, leftEndPoint, /*isLeftType=*/true,
        /*isFront=*/true, LeftPointRailNodeName + MiddleSwitchSuffix);
    if (leftPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToRightLeft,
            +InternalSplineType::RailToLeftLeft, leftEndPoint, false);
    }

    if (leftPointRail && rightPointRail)
    {
        generateSwitchRods(sharedData, *leftPointRail, *rightPointRail, MiddleSwitchSuffix,
            RailwayMeshGeneratorConfig::SwitchRodsType::TWSecond);
        generateCommonSwitchBox(Math::lerp(*leftStartTransform, *rightStartTransform, 0.5),
            RailwayMeshGeneratorConfig::SwitchRodsType::TWSecond);
    }
}

void
DynamicRailwayThreeWayJunction::generateRootPointRailsForRightSide(SharedDataArgs& sharedData,
    const RailFrogInfo& firstSideFrog)
{
    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);

    double startDistance = sleeperStep / 2.;

    // Near right

    const auto& railToMiddleRight = getSpline(sharedData, +InternalSplineType::RailToMiddleRight);
    double endDistance = railToMiddleRight.arcLength() - sleeperStep;

    auto rightEndPoints = findPointRailRoot(sharedData, sharedData.pointRailStockOffset,
        +InternalSplineType::RailToMiddleRight, +InternalSplineType::RailToSideRight, startDistance,
        endDistance);
    if (!rightEndPoints)
    {
        return;
    }
    const TransformSpline::Point& rightEndPoint = rightEndPoints->first;

    const auto rightStartTransform = railToMiddleRight.calcSplineMatrixAtDistance(startDistance);
    if (!rightStartTransform)
    {
        return;
    }
    TransformSpline::Point rightStartPoint{startDistance, *rightStartTransform};
    auto rightPointRail = generatePointRail(sharedData, +InternalSplineType::RailToMiddleRight,
        +InternalSplineType::RailToSideRight, rightStartPoint, rightEndPoint, /*isLeftType=*/false,
        /*isFront=*/true, RightPointRailNodeName + NearSwitchSuffix);
    if (rightPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToMiddleRight,
            +InternalSplineType::RailToSideRight, rightEndPoint, true);
    }

    // Near right

    Math::dvec3 from = rightEndPoint.transform.getTranslate();
    Math::dvec3 dir = rightEndPoint.transform.getAxisY();

    auto leftEndPoints = getSplineSearch(sharedData, +InternalSplineType::RailToSideLeft)
                              .findClosestPoints(toGlm(from + dir * sharedData.railOffset),
                                  toGlm(from + dir * sharedData.railOffset * 2.5));
    if (!leftEndPoints)
    {
        return;
    }
    const TransformSpline::Point& leftEndPoint = leftEndPoints->first;

    const auto& railToSideLeft = getSpline(sharedData, +InternalSplineType::RailToSideLeft);
    const auto leftStartTransform = railToSideLeft.calcSplineMatrixAtDistance(startDistance);
    if (!leftStartTransform)
    {
        return;
    }

    TransformSpline::Point leftStartPoint{startDistance, *leftStartTransform};
    auto leftPointRail = generatePointRail(sharedData, +InternalSplineType::RailToSideLeft,
        +InternalSplineType::RailToMiddleLeft, leftStartPoint, leftEndPoint,
        /*isLeftType=*/true, /*isFront=*/true, LeftPointRailNodeName + NearSwitchSuffix);
    if (leftPointRail)
    {
        generateHeelBlock(sharedData, +InternalSplineType::RailToSideLeft,
            +InternalSplineType::RailToMiddleLeft, leftEndPoint, false);
    }

    if (leftPointRail && rightPointRail)
    {
        generateSwitchRods(sharedData, *leftPointRail, *rightPointRail, NearSwitchSuffix,
            RailwayMeshGeneratorConfig::SwitchRodsType::TWFirst);
        generateCommonSwitchBox(Math::lerp(*leftStartTransform, *rightStartTransform, 0.5),
            RailwayMeshGeneratorConfig::SwitchRodsType::TWFirst);
    }
}

void DynamicRailwayThreeWayJunction::generateCommonSwitchBox(const Unigine::Math::Mat4& transform,
    RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType)
{
    const auto& config = m_config->getSwitchRodsData(+switchRodConfigType);

    //коробка стрелочного перевода.
    std::string switchBoxNodePath = config.getSwitchBoxNodeInfo().nodePath;
    if (auto switchBoxNode = MetaUnigine::loadNode(switchBoxNodePath.data()))
    {
        switchBoxNode->setName("AutomaticSwitchBox");
        switchBoxNode->setTransform(transform);
        switchBoxNode->setParent(getMetaObject());
    }
}

void
DynamicRailwayThreeWayJunction::generateSwitchRods(SharedDataArgs& sharedData,
    const PointRailInfo& leftPointRail, const PointRailInfo& rightPointRail,
    const char* switchSuffix, RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType)
{
    message("generating switch rods...\n");

	const auto& config = m_config->getSwitchRodsData(+switchRodConfigType);

    double leftLength =
        std::abs(leftPointRail.end.distanceOffset - leftPointRail.start.distanceOffset);
    double rightLength =
        std::abs(rightPointRail.end.distanceOffset - rightPointRail.start.distanceOffset);
    double pointRailsLength = std::min(leftLength, rightLength);

    auto calcPointRailMiddleTransform = [this, &leftPointRail, &rightPointRail](bool left,
        double offsetFromRoot, const MetaUnigine::NodePtr& scaleNode = {})
    {
        const auto leftStockTrans =
            left ? leftPointRail.leftTransform : leftPointRail.rightTransform; 
        const Math::Vec3 leftOrigin = leftStockTrans.getTranslate();
        const Math::Vec3 leftDirection = leftStockTrans.getAxisX();

        const auto rightStockTrans =
            left ? rightPointRail.leftTransform : rightPointRail.rightTransform;
        const Math::Vec3 rightOrigin = rightStockTrans.getTranslate();
        const Math::Vec3 rightDirection = rightStockTrans.getAxisX();

        const Math::Vec3 leftPosition = leftOrigin + (leftDirection * offsetFromRoot);
        const Math::Vec3 rightPosition = rightOrigin + (rightDirection * offsetFromRoot);

        const Math::Mat4 result = transformFromPositionAndTangent(
            (leftPosition + rightPosition) * 0.5, -(leftDirection + rightDirection).normalize());

        return result;
    };

    auto addRodNode = [this, &sharedData, &config, &calcPointRailMiddleTransform]
    (int rodIndex, double offsetFromRoot, const std::string& rodName, bool scale = true)
    {
        std::string sourceNodePath = config.rodNodePath;
        if (rodIndex == +RailwayMeshGeneratorConfig::SwitchRodIndex::RodHead)
        {
            sourceNodePath = config.getHeadRodNodePath();
        }

        auto rodNode = MetaUnigine::loadNode(sourceNodePath.c_str());
        if (!rodNode)
        {
            error("Can not load rod model from path: %s\n", sourceNodePath.data());
            return;
        }

        const Math::Mat4 leftTransform = calcPointRailMiddleTransform(true, offsetFromRoot,
            scale ? rodNode : MetaUnigine::NodePtr());
        const Math::Mat4 rightTransform = calcPointRailMiddleTransform(false, offsetFromRoot,
            scale ? rodNode : MetaUnigine::NodePtr());

        auto component = MetaUnigine::getOrAddComponent<RailSwitchPartTransforms>(rodNode);
        assert(component);
        component->setLeftTransform(leftTransform);
        component->setRightTransform(rightTransform);

        rodNode->setName(rodName.data());
        rodNode->setTransform(leftTransform);
        rodNode->setParent(getMetaObject());
    };

    // 0. тяга привода
    {
        addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::RodHead, pointRailsLength,
            std::string("RodHead") + switchSuffix);
    }
}

void
DynamicRailwayThreeWayJunction::generateSleepersOnOuts(SharedDataArgs& sharedData)
{
    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);

    if (sharedData.lastSleeperTransform)
    {
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathRootToSide]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };
            generateSleepersOnOut(sharedData, +InternalSplineType::PathRootToSide,
                sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc);
        }
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathMiddleToLeft]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };
            generateSleepersOnOut(sharedData, +InternalSplineType::PathMiddleToLeft,
                sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc);
        }
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathMiddleToRight]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };
            generateSleepersOnOut(sharedData, +InternalSplineType::PathMiddleToRight,
                sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc);
        }
    }
    else
    {
        Log::error("sharedData.lastSleeperTransform == std::nullopt for %d junction \n", m_key.id);
    }
}

void DynamicRailwayThreeWayJunction::addJunctionProperty(SharedDataArgs& sharedData)
{
    message("adding juncton property component...\n");

    int propIndex = getMetaObject()->findProperty("junction");
    if (propIndex < 0)
    {
        propIndex = getMetaObject()->addProperty("junction");

        PropertyPtr props = getMetaObject()->getProperty(propIndex);
        PropertyParameterPtr typeParam = props->getParameterPtr("type");
        typeParam->setValueSwitch(JUNCTION_TYPE_THREEWAY);
    }
}

void
DynamicRailwayThreeWayJunction::addConnectionPoints(SharedDataArgs& sharedData)
{
    message("adding connection points...\n");

    const int JUNCTION_POINT_CENTRAL = 0;
    const int JUNCTION_POINT_INPUT = 1;
    const int JUNCTION_POINT_SIDE = 2;
    const int JUNCTION_POINT_LEFT = 3;
    const int JUNCTION_POINT_RIGHT = 4;
    const int JUNCTION_POINT_TOTAL = 5;

    Math::Vec3 points[JUNCTION_POINT_TOTAL];

    const auto& pathRootToMiddleSegments =
        getSpline(sharedData, +InternalSplineType::PathRootToMiddle).getSegmentsMap();
    points[JUNCTION_POINT_INPUT] = fromGlm(pathRootToMiddleSegments(0)->p0());
    points[JUNCTION_POINT_CENTRAL] =
        fromGlm(pathRootToMiddleSegments(pathRootToMiddleSegments.iterative_size() - 1)->p3());

    const auto& pathRootToSideSegments =
        getSpline(sharedData, +InternalSplineType::PathRootToSide).getSegmentsMap();
    points[JUNCTION_POINT_SIDE] =
        fromGlm(pathRootToSideSegments(pathRootToSideSegments.iterative_size() - 1)->p3());

    const auto& pathMiddleToLeftSegments =
        getSpline(sharedData, +InternalSplineType::PathMiddleToLeft).getSegmentsMap();
    points[JUNCTION_POINT_LEFT] =
        fromGlm(pathMiddleToLeftSegments(pathMiddleToLeftSegments.iterative_size() - 1)->p3());

    const auto& pathMiddleToRightSegments =
        getSpline(sharedData, +InternalSplineType::PathMiddleToRight).getSegmentsMap();
    points[JUNCTION_POINT_RIGHT] =
        fromGlm(pathMiddleToRightSegments(pathMiddleToRightSegments.iterative_size() - 1)->p3());

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

bool
DynamicRailwayThreeWayJunction::isLeftSideJunction(SharedDataArgs& sharedData)
{
    const auto& sideSpline = getSpline(sharedData, +InternalSplineType::PathRootToSide);
    auto sideStart = sideSpline.pointAtDistance(0.0);
    auto sideEnd = sideSpline.pointAtDistance(sideSpline.arcLength());
    if (!sideStart || !sideEnd)
    {
        error("Unable to find side spline edges\n");
        return false;
    }
    auto sideVec = *sideEnd - *sideStart;

    const auto& leftSpline = getSpline(sharedData, +InternalSplineType::PathMiddleToLeft);
    auto leftStart = leftSpline.pointAtDistance(0.0);
    auto leftEnd = leftSpline.pointAtDistance(leftSpline.arcLength());
    if (!leftStart || !leftEnd)
    {
        error("Unable to find left spline edges\n");
        return false;
    }
    auto leftVec = *leftEnd - *leftStart;

#ifdef DYNAMIC_RAILWAYS_DEBUG
    // Debug code for input spline layout checking
    //m_debugVisualizer = std::make_shared<ObjectDebugVisualizer>();
    //if (m_debugVisualizer)
    //{
    //    m_debugVisualizer->addVector(fromGlm(*sideStart), fromGlm(*sideEnd), Math::vec4::BLUE);
    //    m_debugVisualizer->addVector(fromGlm(*leftStart), fromGlm(*leftEnd));
    //}
#endif

    if (glm::dot(sideVec, leftVec) < 0.0)
    {
        error("Opposite paths are not supported\n");
        return false;
    }

    auto up = glm::cross(leftVec, sideVec);
    if (up.z < 0.0)
    {
        return false;
    }
    return true;
}

bool
DynamicRailwayThreeWayJunction::isRightSideJunction(SharedDataArgs& sharedData)
{
    const auto& rightSpline = getSpline(sharedData, +InternalSplineType::PathMiddleToRight);
    auto rightStart = rightSpline.pointAtDistance(0.0);
    auto rightEnd = rightSpline.pointAtDistance(rightSpline.arcLength());
    if (!rightStart || !rightEnd)
    {
        error("Unable to find right spline edges\n");
        return false;
    }
    auto rightVec = *rightEnd - *rightStart;

    const auto& sideSpline = getSpline(sharedData, +InternalSplineType::PathRootToSide);
    auto sideStart = sideSpline.pointAtDistance(0.0);
    auto sideEnd = sideSpline.pointAtDistance(sideSpline.arcLength());
    if (!sideStart || !sideEnd)
    {
        error("Unable to find side spline edges\n");
        return false;
    }
    auto sideVec = *sideEnd - *sideStart;

    if (glm::dot(sideVec, rightVec) < 0.0)
    {
        error("Opposite paths are not supported\n");
        return false;
    }

    auto up = glm::cross(sideVec, rightVec);
    if (up.z < 0.0)
    {
        return false;
    }
    return true;

}
