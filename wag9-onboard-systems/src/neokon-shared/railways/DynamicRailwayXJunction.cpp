#include "DynamicRailwayXJunction.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/Math/WrapValueAround.h"
#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/NormalizedAngleFromTo.h"
#include "neokon-shared/railways/RailSwitchInfoComponents.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConstants.h"
#include "neokon-shared/StringFormat.h"
#include "neokon-shared/TransformFromPositionAndAxis.h"
#include "neokon-shared/TransformFromTwoPointPairs.h"

using namespace Unigine;

namespace
{
const char* FrontPointRailNodeName = "PointRailCross_Anim1";
const char* BackPointRailNodeName = "PointRailCross_Anim2";
const char* FrontSwitchSuffix = "Cross_Anim1";
const char* BackSwitchSuffix = "Cross_Anim2";
const char* LeftFrontSwitchSuffix = "LeftCross_Anim1";
const char* RightFrontSwitchSuffix = "RightCross_Anim1";
const char* LeftBackSwitchSuffix = "LeftCross_Anim2";
const char* RightBackSwitchSuffix = "RightCross_Anim2";

const char* SnapToPrefix = "snapTo:";
} // Anonymous namespace

DynamicRailwayXJunction::DynamicRailwayXJunction(uint32_t partId, const std::string& configPath,
    std::vector<BezierSpline>&& splines,
    std::vector<SplineNeighborConfigs>&& neighborConfigs,
    const Unigine::Math::Mat4& transform, bool splitSwitch)
    : DynamicRailwayJunctionBase(partId, configPath, std::move(splines),
        std::move(neighborConfigs), transform)
    , m_splitSwitch(splitSwitch)
{
}

bool
DynamicRailwayXJunction::load()
{
    if (!loadConfig())
    {
        return false;
    }

    setMetaObject(MetaUnigine::ObjectMeshStatic::create());
    getMetaObject()->setWorldTransform(getTransform());

    auto mesh = Mesh::create();

    SharedDataArgs sharedData{mesh};
    if (!buildSplines(sharedData))
    {
        return false;
    }

    generateEmbankment(sharedData);

    //верхняя тупая крестовина
    auto rightFrog = generateRailFrog(sharedData, +InternalSplineType::RailClosureFrontLeft,
        +InternalSplineType::RailClosureBackLeft, /*flip=*/false, /*isObtuse=*/true);
    //нижняя тупая крестовина
    auto leftFrog = generateRailFrog(sharedData, +InternalSplineType::RailClosureBackRight,
        +InternalSplineType::RailClosureFrontRight, /*flip=*/true, /*isObtuse=*/true);
    if (!leftFrog || !rightFrog)
    {
        return false;
    }
    bool isLeftNarrow = isLeftNarrowJunction(sharedData, leftFrog->transform, rightFrog->transform);
    bool isRightNarrow =
        isRightNarrowJunction(sharedData, leftFrog->transform, rightFrog->transform);

    if (isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::JunctionRails))
    {
        // передняя острая крестовина
        auto frontFrog = generateRailFrog(sharedData, +InternalSplineType::RailClosureFrontLeft,
            +InternalSplineType::RailClosureFrontRight, /*flip=*/false, /*isObtuse=*/false);
        // задняя острая крестовина
        auto backFrog = generateRailFrog(sharedData, +InternalSplineType::RailClosureBackRight,
            +InternalSplineType::RailClosureBackLeft, /*flip=*/true, /*isObtuse=*/false);

        if (frontFrog)
        {
            generateGuardRailsForRailFrog(sharedData, *frontFrog,
                sharedData.guardRailClosureOffset);
            if (isLeftNarrow || isRightNarrow)
            {
                generateNarrowPointRailsForSharpFrog(sharedData, *frontFrog, *leftFrog, *rightFrog);
            }
        }

        if (backFrog)
        {
            generateGuardRailsForRailFrog(sharedData, *backFrog, sharedData.guardRailClosureOffset);
            if (isLeftNarrow || isRightNarrow)
            {
                generateNarrowPointRailsForSharpFrog(sharedData, *backFrog, *leftFrog, *rightFrog);
            }
        }

        m_splitSwitch = (!isLeftNarrow && !isRightNarrow);

        if (!isLeftNarrow)
        {
            generateRailFrog(sharedData, +InternalSplineType::RailClosureFrontLeft,
                +InternalSplineType::RailMiddleRightToRight, /*flip=*/false, /*isObtuse=*/false);
            generateRailFrog(sharedData, +InternalSplineType::RailClosureBackLeft,
                +InternalSplineType::RailMiddleRightToRight, /*flip=*/true, /*isObtuse=*/false);

            generateWidePointRails(sharedData, +InternalSplineType::RailMiddleRightToRight,
                +InternalSplineType::RailClosureBackRight, /*pointSplineIsLeft=*/true,
                RailwayMeshGeneratorConfig::SwitchRodsType::XFrontLeft);
            generateWidePointRails(sharedData, +InternalSplineType::RailMiddleRightToRight,
                +InternalSplineType::RailClosureFrontRight, /*pointSplineIsLeft=*/true,
                RailwayMeshGeneratorConfig::SwitchRodsType::XBackLeft);
        }

        if (!isRightNarrow)
        {
            generateRailFrog(sharedData, +InternalSplineType::RailClosureFrontRight,
                +InternalSplineType::RailMiddleLeftToLeft, /*flip=*/false, /*isObtuse=*/false);
            generateRailFrog(sharedData, +InternalSplineType::RailClosureBackRight,
                +InternalSplineType::RailMiddleLeftToLeft, /*flip=*/true, /*isObtuse=*/false);

            generateWidePointRails(sharedData, +InternalSplineType::RailMiddleLeftToLeft,
                +InternalSplineType::RailClosureBackLeft, /*pointSplineIsLeft=*/false,
                RailwayMeshGeneratorConfig::SwitchRodsType::XFrontRight);
            generateWidePointRails(sharedData, +InternalSplineType::RailMiddleLeftToLeft,
                +InternalSplineType::RailClosureFrontLeft, /*pointSplineIsLeft=*/false,
                RailwayMeshGeneratorConfig::SwitchRodsType::XBackRight);
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
    generateMiddleSleepers(sharedData, sharedData.middleSpline, interFillVec(+InternalSplineType::RailStockLeft,
        +InternalSplineType::RailStockRight), addMiddleSleeperInfoFunc);

    generateSleepersOnOuts(sharedData, isLeftNarrow, isRightNarrow);
    setSleepers(std::move(sharedData.sleepersInfos));

    mesh->createBounds();
    getMetaObject()->setMesh(mesh);

    addJunctionProperty(sharedData);
    addConnectionPoints(sharedData);
    setupMeshMaterials(sharedData);

    return true;
}

bool
DynamicRailwayXJunction::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto part = std::dynamic_pointer_cast<DynamicRailwayXJunction>(old);
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
DynamicRailwayXJunction::createNodes()
{
    auto result = DynamicRailwayJunctionBase::createNodes();
    if (result)
    {
        // Model is generated switched right to right, so switch it here if other was requested
        // before generation
        SwitchState frontSwitchState = m_frontLeftSwitchState;
        SwitchState frontRightSwitchState = m_frontRightSwitchState;
        SwitchState backSwitchState = m_backLeftSwitchState;
        SwitchState backRightSwitchState = m_backRightSwitchState;

        m_frontRightSwitchState = m_backRightSwitchState = SwitchState::Right;
        m_frontLeftSwitchState = m_backLeftSwitchState = SwitchState::Right;

        if (m_splitSwitch)
        {
            doSwitch4(frontSwitchState, frontRightSwitchState, backSwitchState,
                backRightSwitchState);
        }
        else
        {
            doSwitch2(frontSwitchState, backSwitchState);
        }
    }
    return result;
}

void
DynamicRailwayXJunction::switchStraightDOnly()
{
    doSwitch4(Left, Right, Left, Right);
}

void
DynamicRailwayXJunction::switchLeftToRight()
{
    doSwitch2(SwitchState::Right, SwitchState::Left);
}

void
DynamicRailwayXJunction::switchRightToLeft()
{
    doSwitch2(SwitchState::Left, SwitchState::Right);
}

void
DynamicRailwayXJunction::switchLeftToLeftXOnly()
{
    doSwitch2(SwitchState::Left, SwitchState::Left);
}

void
DynamicRailwayXJunction::switchRightToRightXOnly()
{
    doSwitch2(SwitchState::Right, SwitchState::Right);
}

void
DynamicRailwayXJunction::doSwitch2(SwitchState front, SwitchState back)
{
    if (m_splitSwitch)
    {
        doSwitch4(front, front, back, back);
        return;
    }

    if (m_frontLeftSwitchState != front)
    {
        if (auto model = getCreatedNodes())
        {
            if (front == SwitchState::Right)
            {
                railSwitchToLeft(model, FrontSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, FrontSwitchSuffix);
            }
        }
        m_frontLeftSwitchState = m_frontRightSwitchState = front;
    }

    if (m_backLeftSwitchState != back)
    {
        if (auto model = getCreatedNodes())
        {
            if (back == SwitchState::Right)
            {
                railSwitchToLeft(model, BackSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, BackSwitchSuffix);
            }
        }
        m_backLeftSwitchState = m_backRightSwitchState = back;
    }
}

void DynamicRailwayXJunction::doSwitch4(SwitchState frontLeft, SwitchState frontRight,
    SwitchState backLeft, SwitchState backRight)
{
    if (!m_splitSwitch)
    {
        return;
    }

    if (m_frontLeftSwitchState != frontLeft)
    {
        if (auto model = getCreatedNodes())
        {
            if (frontLeft == SwitchState::Right)
            {
                railSwitchToLeft(model, LeftFrontSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, LeftFrontSwitchSuffix);
            }
        }
        m_frontLeftSwitchState = frontLeft;
    }

    if (m_frontRightSwitchState != frontRight)
    {
        if (auto model = getCreatedNodes())
        {
            if (frontRight == SwitchState::Right)
            {
                railSwitchToLeft(model, RightFrontSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, RightFrontSwitchSuffix);
            }
        }
        m_frontRightSwitchState = frontRight;
    }

    if (m_backLeftSwitchState != backLeft)
    {
        if (auto model = getCreatedNodes())
        {
            if (backLeft == SwitchState::Right)
            {
                railSwitchToLeft(model, LeftBackSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, LeftBackSwitchSuffix);
            }
        }
        m_backLeftSwitchState = backLeft;
    }

    if (m_backRightSwitchState != backRight)
    {
        if (auto model = getCreatedNodes())
        {
            if (backRight == SwitchState::Right)
            {
                railSwitchToLeft(model, RightBackSwitchSuffix);
            }
            else
            {
                railSwitchToRight(model, RightBackSwitchSuffix);
            }
        }
        m_backRightSwitchState = backRight;
    }
}

//------------------------------------------------------------------------------------------
// Generation steps
//------------------------------------------------------------------------------------------
bool
DynamicRailwayXJunction::buildSplines(SharedDataArgs& sharedData)
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

    // paths for sleepers on outs
    sharedData.splineSearchById[+InternalSplineType::PathLeftToRight].build(
        getSpline(sharedData, +InternalSplineType::PathLeftToRight));
    sharedData.splineSearchById[+InternalSplineType::PathRightToLeft].build(
        getSpline(sharedData, +InternalSplineType::PathRightToLeft));

    // left stock rail
    addRail(sharedData, +InternalSplineType::PathLeftToLeft, +InternalSplineType::RailStockLeft,
        "RailStockLeft", railOffset, /*mirror=*/false);

    // left middle rail
    addRail(sharedData, +InternalSplineType::PathRightToRight,
        +InternalSplineType::RailMiddleRightToRight, "RailMiddleRightToRight", railOffset, /*mirror=*/false);

    // right stock rail
    addRail(sharedData, +InternalSplineType::PathRightToRight, +InternalSplineType::RailStockRight,
        "RailStockRight", -railOffset,
        /*mirror=*/true);

    // right middle rail
    addRail(sharedData, +InternalSplineType::PathLeftToLeft,
        +InternalSplineType::RailMiddleLeftToLeft,
        "RailMiddleLeftToLeft", -railOffset,
        /*mirror=*/true);

    // front closure rail of left path
    addRail(sharedData, +InternalSplineType::PathLeftToRight,
        +InternalSplineType::RailClosureFrontLeft, "RailClosureFrontLeft", -railOffset,
        /*mirror=*/true);

    // front closure rail of right path
    addRail(sharedData, +InternalSplineType::PathRightToLeft,
        +InternalSplineType::RailClosureFrontRight, "RailClosureFrontRight", railOffset,
        /*mirror=*/false);

    // back closure rail of left path
    addRail(sharedData, +InternalSplineType::PathRightToLeft,
        +InternalSplineType::RailClosureBackLeft, "RailClosureBackLeft", -railOffset,
        /*mirror=*/true);

    // back closure rail of right path
    addRail(sharedData, +InternalSplineType::PathLeftToRight,
        +InternalSplineType::RailClosureBackRight, "RailClosureBackRight", railOffset,
        /*mirror=*/false);

    //Ищем геометрический центр стрелки, который располагается на
    //пересечении перекрестных путей. Метод findPointsWithDistance
    //поиска на сплайнах, находит ближайшие точки на двух сплайнах.
    //Если расстояние между ними меньше 10 см (0.1 м), мы предполагаем, что
    //найденная пара - искомое пересечение.

    auto const& leftPathSpline = getSpline(sharedData, +InternalSplineType::PathLeftToRight);
    TransformSpline leftPathSearch;
    leftPathSearch.build(leftPathSpline);

    auto const& rightPathSpline = getSpline(sharedData, +InternalSplineType::PathRightToLeft);
    //ищем пару точек между правым и левым сплайном, которые отстоят друг от друга на расстоянии
    //макисмально близком заданной как второй параметр дистанции (0.0 метра).
    auto crossPoint = leftPathSearch.findPointsWithDistance(rightPathSpline, 0.0);
    if (!crossPoint || crossPoint->gap > 0.1)
    {
        error("Unable to find closure cross point\n");
        return false;
    }

    double distanceDelta = crossPoint->second.distanceOffset - crossPoint->first.distanceOffset;
    if (distanceDelta > 0.0)
    {
        sharedData.middleSpline.build(
            leftPathSpline, rightPathSpline, /*k=*/[](double) { return 0.5; }, /*step=*/0.1,
            /*firstStartM=*/0.0, /*secondStartM=*/distanceDelta);
    }
    else
    {
        sharedData.middleSpline.build(
            leftPathSpline, rightPathSpline, /*k=*/[](double) { return 0.5; }, /*step=*/0.1,
            /*firstStartM=*/-distanceDelta, /*secondStartM=*/0.0);
    }

    auto closureCrossPointPosition = crossPoint->first.transform.getTranslate();
    auto middleCrossPoint =
        sharedData.middleSpline.findClosestPoint(toGlm(closureCrossPointPosition));
    if (!middleCrossPoint)
    {
        error("Unable to find middle cross point\n");
        return false;
    }

    //Оцениваем рассотяние между осью стрелки (сплайн sharedData.middleSpline)
    //и точкой пересечения путей (closureCrossPointPosition). Если ось стрелки не проходит,
    //через пересечение путей (ближайшая пара точек отстоит друг от друга больше чем на 0.2м),
    //то мы не сможем корректно расположить брусья (sleepers).
    double middleCrossError =
        (middleCrossPoint->transform.getTranslate() - closureCrossPointPosition).length();
    if (middleCrossError > 0.2)
    {
        warning("Too big distance between sleepers and closure spline's cross points == %f\n",
            middleCrossError);
    }

    sharedData.middleCrossPoint = *middleCrossPoint;

    if (getSpline(sharedData, +InternalSplineType::RailStockLeft).empty())
    {
        sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::RailClosureFrontRight);
        sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::RailClosureBackRight);
    }
    else
    {
        sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::RailStockLeft);
    }

    if (getSpline(sharedData, +InternalSplineType::RailStockRight).empty())
    {
        sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::RailClosureFrontLeft);
        sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::RailClosureBackLeft);
    }
    else
    {
        sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::RailStockRight);
    }

    sharedData.oppositeSplineIds = {
        {+InternalSplineType::RailClosureFrontLeft, +InternalSplineType::RailClosureBackRight},
        {+InternalSplineType::RailClosureFrontRight, +InternalSplineType::RailClosureBackLeft},
        {+InternalSplineType::RailClosureBackRight, +InternalSplineType::RailClosureFrontLeft},
        {+InternalSplineType::RailClosureBackLeft, +InternalSplineType::RailClosureFrontRight},
        {+InternalSplineType::RailMiddleRightToRight, +InternalSplineType::RailStockRight},
        {+InternalSplineType::RailMiddleLeftToLeft, +InternalSplineType::RailStockLeft}
    };

    return true;
}

void
DynamicRailwayXJunction::generateNarrowPointRailsForSharpFrog(SharedDataArgs& sharedData,
    const RailFrogInfo& sharpFrog, const RailFrogInfo& leftFrog, const RailFrogInfo& rightFrog)
{
    auto getEndDistance = [&sharpFrog, &leftFrog, &rightFrog](int splineIdx)
    {
        auto selectFromInterval = [&sharpFrog](const std::pair<double, double>& interval)
        {
            if (sharpFrog.isFlipped)
            {
                return std::max(interval.first, interval.second);
            }
            return std::min(interval.first, interval.second);
        };

        // Offset from frog, to give space for heel blocks not to be placed on the frog.
        // Also fixes problem with overlapping of non-closure point rail on the frog because frogs
        // are aligned perpendiculary to junction middle axis - point rails are rotated to sides
        double safeOffsetM = sharpFrog.isFlipped ? 0.3 : -0.3;
        if (splineIdx == leftFrog.leftInputSplineIdx)
        {
            if (!leftFrog.leftInputSplineInterval)
            {
                return leftFrog.crossPoint.first.distanceOffset + safeOffsetM;
            }
            return selectFromInterval(*leftFrog.leftInputSplineInterval) + safeOffsetM;
        }
        if (splineIdx == leftFrog.rightInputSplineIdx)
        {
            if (!leftFrog.rightInputSplineInterval)
            {
                return leftFrog.crossPoint.second.distanceOffset + safeOffsetM;
            }
            return selectFromInterval(*leftFrog.rightInputSplineInterval) + safeOffsetM;
        }
        if (splineIdx == rightFrog.leftInputSplineIdx)
        {
            if (!rightFrog.leftInputSplineInterval)
            {
                return rightFrog.crossPoint.first.distanceOffset + safeOffsetM;
            }
            return selectFromInterval(*rightFrog.leftInputSplineInterval) + safeOffsetM;
        }
        if (splineIdx == rightFrog.rightInputSplineIdx)
        {
            if (!rightFrog.rightInputSplineInterval)
            {
                return rightFrog.crossPoint.second.distanceOffset + safeOffsetM;
            }
            return selectFromInterval(*rightFrog.rightInputSplineInterval) + safeOffsetM;
        }
        throw std::out_of_range(
            stringFormat("getEndDistance(%d) failed, spline not found in both frogs", splineIdx));
        return 0.0;
    };

    const char* pointRailNodeName =
        sharpFrog.isFlipped ? BackPointRailNodeName : FrontPointRailNodeName;

    // sharpFrog.leftInputSplineIdx - is left front, which goes to right after frog
    if (auto closurePoints = findMiddlePointRailsStart(sharedData, sharpFrog))
    {
        NarrowPointRails pointRails;

        const auto& leftClosurePoint =
            sharpFrog.isFlipped ? closurePoints->first : closurePoints->second;
        auto leftMiddleStart =
            getSplineSearch(sharedData, +InternalSplineType::RailMiddleRightToRight)
                .findClosestPoint(toGlm(leftClosurePoint.transform.getTranslate()));
        int leftOutputSplineIdx =
            sharpFrog.isFlipped ? sharpFrog.leftInputSplineIdx : sharpFrog.rightInputSplineIdx;
        if (leftMiddleStart)
        {
            double endDistance = getEndDistance(leftOutputSplineIdx);
            if (auto transform = getSpline(sharedData, leftOutputSplineIdx)
                                     .calcSplineMatrixAtDistance(endDistance))
            {
                auto leftMiddleEnd =
                    getSplineSearch(sharedData, +InternalSplineType::RailMiddleRightToRight)
                        .findClosestPoint(toGlm(transform->getTranslate()));
                if (leftMiddleEnd)
                {
                    if (auto pointRail = generatePointRail(sharedData,
                            +InternalSplineType::RailMiddleRightToRight, leftOutputSplineIdx,
                            *leftMiddleStart, *leftMiddleEnd, !sharpFrog.isFlipped,
                            !sharpFrog.isFlipped, pointRailNodeName))
                    {
                        pointRails[+NarrowPointRailIndex::LeftMiddle] = *pointRail;
                        generateHeelBlock(sharedData, +InternalSplineType::RailMiddleRightToRight,
                            leftOutputSplineIdx, *leftMiddleEnd, true);
                    }
                }

                auto oppositeStart = findOppositeRailPoint(sharedData, leftOutputSplineIdx,
                    /*isLeft=*/true, leftMiddleStart->transform);
                auto oppositeEnd = findOppositeRailPoint(sharedData, leftOutputSplineIdx,
                    /*isLeft=*/true, *transform);
                if (oppositeStart && oppositeEnd)
                {
                    auto splineIdx = getOppositeSplineId(sharedData, leftOutputSplineIdx);
                    assert(splineIdx != std::nullopt);
                    if (auto pointRail = generatePointRail(sharedData, *splineIdx,
                            +InternalSplineType::RailStockRight, *oppositeStart, *oppositeEnd,
                            sharpFrog.isFlipped, !sharpFrog.isFlipped, pointRailNodeName))
                    {
                        pointRails[+NarrowPointRailIndex::RightStock] = *pointRail;
                        generateHeelBlock(sharedData, *splineIdx,
                            +InternalSplineType::RailStockRight, *oppositeEnd, false);
                    }
                }
            }
        }

        const auto& rightClosurePoint =
            sharpFrog.isFlipped ? closurePoints->second : closurePoints->first;
        auto rightMiddleStart =
            getSplineSearch(sharedData, +InternalSplineType::RailMiddleLeftToLeft)
                .findClosestPoint(toGlm(rightClosurePoint.transform.getTranslate()));
        int rightOutputSplineIdx =
            sharpFrog.isFlipped ? sharpFrog.rightInputSplineIdx : sharpFrog.leftInputSplineIdx;
        if (rightMiddleStart)
        {
            double endDistance = getEndDistance(rightOutputSplineIdx);
            if (auto transform = getSpline(sharedData, rightOutputSplineIdx)
                                     .calcSplineMatrixAtDistance(endDistance))
            {
                auto rightMiddleEnd =
                    getSplineSearch(sharedData, +InternalSplineType::RailMiddleLeftToLeft)
                        .findClosestPoint(toGlm(transform->getTranslate()));
                if (rightMiddleEnd)
                {
                    if (auto pointRail =
                            generatePointRail(sharedData, +InternalSplineType::RailMiddleLeftToLeft,
                                rightOutputSplineIdx, *rightMiddleStart, *rightMiddleEnd,
                                sharpFrog.isFlipped, !sharpFrog.isFlipped, pointRailNodeName))
                    {
                        pointRails[+NarrowPointRailIndex::RightMiddle] = *pointRail;
                        generateHeelBlock(sharedData, +InternalSplineType::RailMiddleLeftToLeft,
                            rightOutputSplineIdx, *rightMiddleEnd, false);
                    }
                }

                auto oppositeStart = findOppositeRailPoint(sharedData, rightOutputSplineIdx,
                    /*isLeft=*/false, rightMiddleStart->transform);
                auto oppositeEnd = findOppositeRailPoint(sharedData, rightOutputSplineIdx,
                    /*isLeft=*/false, *transform);
                if (oppositeStart && oppositeEnd)
                {
                    auto splineIdx = getOppositeSplineId(sharedData, rightOutputSplineIdx);
                    assert(splineIdx != std::nullopt);
                    if (auto pointRail = generatePointRail(sharedData, *splineIdx,
                            +InternalSplineType::RailStockLeft, *oppositeStart, *oppositeEnd,
                            !sharpFrog.isFlipped, !sharpFrog.isFlipped, pointRailNodeName))
                    {
                        pointRails[+NarrowPointRailIndex::LeftStock] = *pointRail;
                        generateHeelBlock(sharedData, *splineIdx,
                            +InternalSplineType::RailStockLeft, *oppositeEnd, true);
                    }
                }
            }
        }

        generateNarrowSwitchRods(sharedData, pointRails, sharpFrog.isFlipped);

        std::optional<bool> forceLeftBox;
        if (!pointRails[+NarrowPointRailIndex::LeftStock])
        {
            forceLeftBox = sharpFrog.isFlipped;
        }
        if (!pointRails[+NarrowPointRailIndex::RightStock])
        {
            forceLeftBox = !sharpFrog.isFlipped;
        }

        generateNarrowSwitchBox(
            Math::lerp(closurePoints->first.transform, closurePoints->second.transform, 0.5),
            sharpFrog.isFlipped, forceLeftBox);

        generateNarrowBorderBar(pointRails[+NarrowPointRailIndex::LeftStock],
            pointRails[+NarrowPointRailIndex::LeftMiddle], sharpFrog.isFlipped);

        generateNarrowBorderBar(pointRails[+NarrowPointRailIndex::RightMiddle],
            pointRails[+NarrowPointRailIndex::RightStock], sharpFrog.isFlipped);
    }
}

void
DynamicRailwayXJunction::generateWidePointRails(SharedDataArgs& sharedData, int pointSplineIdx1,
    int joinSplineIdx1, bool pointSplineIsLeft,
    RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType)
{
    message("generating point rails...\n");

    auto pointJoinCross1 = getSplineSearch(sharedData, pointSplineIdx1)
                               .findPointsWithDistance(getSpline(sharedData, joinSplineIdx1), 0.0);
    if (!pointJoinCross1)
    {
        error("Unable to find point rail and join rail cross point 1\n");
        return;
    }

    auto pointSplineIdx2 = getOppositeSplineId(sharedData, joinSplineIdx1);
    auto joinSplineIdx2 = getOppositeSplineId(sharedData, pointSplineIdx1);
    if (!pointSplineIdx2 || !joinSplineIdx2)
    {
        error("Unable to find opposite splines for wide point rails\n");
        return;
    }

    auto pointJoinCross2 = findOppositeRailPoint(sharedData, joinSplineIdx1,
        /*isLeft=*/pointSplineIsLeft, pointJoinCross1->second.transform);
    if (!pointJoinCross2)
    {
        error("Unable to find point rail and base rail cross point 2\n");
        return;
    }

    auto pointRailsFrog = findRailFrogBySplines(sharedData, pointSplineIdx1, *pointSplineIdx2);
    if (!pointRailsFrog)
    {
        error("Unable to find frog between point rails\n");
        return;
    }

    auto getEndDistance = [](const DynamicRailwayJunctionBase::RailFrogInfo& railFrog,
            int splineId, double distance)
    {
        auto nearest = [distance](const DynamicRailwayJunctionBase::SplineInterval& interval) {
            return std::abs(interval.first - distance) > std::abs(interval.second - distance)
                ? interval.second
                : interval.first;
        };

        if (splineId == railFrog.leftInputSplineIdx)
        {
            return railFrog.leftInputSplineInterval ? nearest(*railFrog.leftInputSplineInterval)
                                                    : railFrog.crossPoint.first.distanceOffset;
        }
        if (splineId == railFrog.rightInputSplineIdx)
        {
            return railFrog.rightInputSplineInterval ? nearest(*railFrog.rightInputSplineInterval)
                                                     : railFrog.crossPoint.second.distanceOffset;
        }
        return 0.0;
    };

    double pointRailStartDst1 = pointJoinCross1->first.distanceOffset;
    double pointRailStartDst2 = pointJoinCross2->distanceOffset;

    auto pointRail1NearestFrog = findFirstRailFrog(sharedData, pointSplineIdx1, pointRailStartDst1,
        getEndDistance(*pointRailsFrog, pointSplineIdx1, pointRailStartDst1));
    auto pointRail2NearestFrog = findFirstRailFrog(sharedData, *pointSplineIdx2, pointRailStartDst2,
        getEndDistance(*pointRailsFrog, *pointSplineIdx2, pointRailStartDst2));
    if (!pointRail1NearestFrog || !pointRail2NearestFrog)
    {
        error("Unable to find neares frog for point rail\n");
        return;
    }
    // TODO: Add check for first guard rail to prevent overlapping too 

    double pointRailEndDst1 = getEndDistance(*pointRail1NearestFrog, pointSplineIdx1,
        pointRailStartDst1);
    double pointRailEndDst2 = getEndDistance(*pointRail2NearestFrog, *pointSplineIdx2,
        pointRailStartDst2);

    const double pointRailStockOffset =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getPointRailsData().stockOffset);

    auto pointBaseRoot1 = getSplineSearch(sharedData, pointSplineIdx1)
                              .findPointsWithDistance(getSpline(sharedData, joinSplineIdx1),
                                  pointRailStockOffset + sharedData.railProfileWidth);
    if (pointBaseRoot1 && (std::abs(pointRailEndDst1 - pointRailStartDst1) >
            std::abs(pointBaseRoot1->first.distanceOffset - pointRailStartDst1)))
    {
        pointRailEndDst1 = pointBaseRoot1->first.distanceOffset;
    }

    auto pointBaseRoot2 = getSplineSearch(sharedData, *pointSplineIdx2)
                              .findPointsWithDistance(getSpline(sharedData, *joinSplineIdx2),
                                  pointRailStockOffset + sharedData.railProfileWidth);
    if (pointBaseRoot2 && (std::abs(pointRailEndDst2 - pointRailStartDst2) >
            std::abs(pointBaseRoot2->first.distanceOffset - pointRailStartDst2)))
    {
        pointRailEndDst2 = pointBaseRoot2->first.distanceOffset;
    }

    TransformSpline::Point pointRailEndPoint1{pointRailEndDst1,
        *getSpline(sharedData, pointSplineIdx1).calcSplineMatrixAtDistance(pointRailEndDst1)};
    TransformSpline::Point pointRailEndPoint2{pointRailEndDst2,
        *getSpline(sharedData, *pointSplineIdx2).calcSplineMatrixAtDistance(pointRailEndDst2)};

    auto toStart = (pointJoinCross1->first.transform.getTranslate() -
        pointRailEndPoint1.transform.getTranslate()).normalize();

    if (!pointBaseRoot1)
    {
        error("Unable to find pointBaseRoot1\n");
        return;
    }
    auto toBase = (pointBaseRoot1->second.transform.getTranslate() -
        pointRailEndPoint1.transform.getTranslate()).normalize();

    bool isLeftType = (Math::cross(toBase, toStart).z > 0);
    bool isFront = (pointJoinCross1->first.distanceOffset < pointRailEndPoint1.distanceOffset);

    std::string nodeNameSuffix = isFront ? FrontSwitchSuffix : BackSwitchSuffix;
    if (m_splitSwitch)
    {
        if (isFront)
        {
            nodeNameSuffix = (isLeftType ^ isFront ? LeftFrontSwitchSuffix : RightFrontSwitchSuffix);
        }
        else
        {
            nodeNameSuffix = (isLeftType ^ isFront ? LeftBackSwitchSuffix : RightBackSwitchSuffix);
        }
    }

    std::string nodeName = "widePointRail" + nodeNameSuffix;

    auto leftPointRail = generatePointRail(sharedData, pointSplineIdx1, joinSplineIdx1,
        pointJoinCross1->first, pointRailEndPoint1, isLeftType, isFront, nodeName.data(), true);

    generateHeelBlock(sharedData, pointSplineIdx1, joinSplineIdx1, pointRailEndPoint1,
        !(isLeftType ^ isFront));

    auto rightPointRail = generatePointRail(sharedData, *pointSplineIdx2, *joinSplineIdx2,
        *pointJoinCross2, pointRailEndPoint2, !isLeftType, isFront, nodeName.data(), true);

     generateHeelBlock(sharedData, *pointSplineIdx2, *joinSplineIdx2, pointRailEndPoint2,
        isLeftType ^ isFront);

    if (!leftPointRail || !rightPointRail)
    {
        return;
    }

    if (!leftPointRail->get().isLeft && rightPointRail->get().isLeft)
    {
        std::swap(leftPointRail, rightPointRail);
    }

    generateSwitchRods(sharedData, *leftPointRail, *rightPointRail, nodeNameSuffix.data(),
        /*initialRightTransform=*/false, switchRodConfigType);

    return;
}

void
DynamicRailwayXJunction::generateNarrowSwitchBox(const Math::Mat4& closureTransform, bool isFlipped,
    std::optional<bool> forceLeft)
{
    const auto& twoSideConfig = m_config->getSwitchRodsData(!isFlipped
            ? +RailwayMeshGeneratorConfig::SwitchRodsType::XFront
            : +RailwayMeshGeneratorConfig::SwitchRodsType::XBack);

    Math::Mat4 boxTransform(closureTransform);
    if (isFlipped)
    {
        boxTransform *= Math::Mat4(Math::quat(0., 0., 180.));
    }

    //коробка стрелочного перевода.
    auto switchBoxNodePath = twoSideConfig.getSwitchBoxNodeInfo(forceLeft).nodePath;
    if (auto switchBoxNode = MetaUnigine::loadNode(switchBoxNodePath.data()))
    {
        switchBoxNode->setName("AutomaticSwitchBox");
        switchBoxNode->setTransform(boxTransform);
        switchBoxNode->setParent(getMetaObject());
    }
}

void
DynamicRailwayXJunction::generateNarrowBorderBar(const std::optional<PointRailInfo>& leftPointRail,
    const std::optional<PointRailInfo>& rightPointRail, bool isFlipped)
{
    if (!leftPointRail || !rightPointRail || !leftPointRail->joinedStartPoint ||
        !rightPointRail->joinedStartPoint)
    {
        return;
    }

    auto transform = Math::lerp(leftPointRail->joinedStartPoint->transform,
        rightPointRail->joinedStartPoint->transform, 0.5);

    const auto& twoSideConfig = m_config->getSwitchRodsData(!isFlipped
            ? +RailwayMeshGeneratorConfig::SwitchRodsType::XFront
            : +RailwayMeshGeneratorConfig::SwitchRodsType::XBack);

    const auto& borderBarPath = twoSideConfig.borderBarPath;
    if (auto borderBarNode = MetaUnigine::loadNode(borderBarPath.data()))
    {
        const auto& sleepersData = m_config->getJunctionSleepersData();

        const double sleeperStep = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.interval);
        const double sleeperWidth =
            RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.sleeperWidth);

        auto offset = sleeperWidth / 2. - sleeperStep * 2.;
        Math::Mat4 addTransform(Math::quat(), Math::Vec3(offset, 0., 0.));
        if (isFlipped)
            addTransform = Math::Mat4(Math::quat(0.0f, 0.0f, 180.0f)) * addTransform;

        auto barTransform = transform * addTransform;

        borderBarNode->setName("BorderBar");
        borderBarNode->setTransform(barTransform);
        borderBarNode->setParent(getMetaObject());
    }
}

void
DynamicRailwayXJunction::generateNarrowSwitchRods(SharedDataArgs& sharedData,
    const NarrowPointRails& pointRails, bool isFlipped)
{
    message("generating switch rods...\n");

    bool singleSide = false;
    std::optional<bool> forceLeft;

    auto leftRail = pointRails[+NarrowPointRailIndex::LeftStock];
    if (!leftRail)
    {
        singleSide = true;
        forceLeft = isFlipped;
        leftRail = pointRails[+NarrowPointRailIndex::LeftMiddle];
    }

    auto rightRail = pointRails[+NarrowPointRailIndex::RightStock];
    if (!rightRail)
    {
        if (singleSide)
        {
            return;
        }
        singleSide = true;
        forceLeft = !isFlipped;
        rightRail = pointRails[+NarrowPointRailIndex::RightMiddle];
    }

    if (!leftRail || !rightRail)
    {
        return;
    }

    const auto& twoSideConfig = m_config->getSwitchRodsData(!isFlipped
            ? +RailwayMeshGeneratorConfig::SwitchRodsType::XFront
            : +RailwayMeshGeneratorConfig::SwitchRodsType::XBack);

    const auto& config = singleSide
        ? m_config->getSwitchRodsData(+RailwayMeshGeneratorConfig::SwitchRodsType::Default)
        : twoSideConfig;

    double leftLength = std::abs(leftRail->end.distanceOffset - leftRail->start.distanceOffset);
    double rightLength = std::abs(rightRail->end.distanceOffset - rightRail->start.distanceOffset);
    double pointRailsLength = std::min(leftLength, rightLength);

    auto calcPointRailMiddleTransform = [this, &leftRail, &rightRail](bool left,
                                            double offsetFromRoot,
                                            const MetaUnigine::NodePtr& scaleNode = {})
    {
        const auto leftStockTrans = left ? leftRail->leftTransform : leftRail->rightTransform;
        const Math::Vec3 leftOrigin = leftStockTrans.getTranslate();
        const Math::Vec3 leftDirection = leftStockTrans.getAxisX();

        const auto rightStockTrans = left ? rightRail->leftTransform : rightRail->rightTransform;
        const Math::Vec3 rightOrigin = rightStockTrans.getTranslate();
        const Math::Vec3 rightDirection = rightStockTrans.getAxisX();

        const Math::Vec3 leftPosition = leftOrigin + (leftDirection * offsetFromRoot);
        const Math::Vec3 rightPosition = rightOrigin + (rightDirection * offsetFromRoot);

        const Math::Mat4 result = transformFromPositionAndTangent(
            (leftPosition + rightPosition) * 0.5, -(leftDirection + rightDirection).normalize());

        return result;
    };

    const char* switchSuffix = !isFlipped ? FrontSwitchSuffix : BackSwitchSuffix;

    auto addRodNode = [this, &sharedData, &pointRails, &config, &calcPointRailMiddleTransform,
                          &switchSuffix, &forceLeft](int rodIndex, double offsetFromRoot,
                          const std::string& rodName, bool scale = true)
    {
        std::string sourceNodePath = config.rodNodePath;
        if (rodIndex == +RailwayMeshGeneratorConfig::SwitchRodIndex::RodHead)
        {
            sourceNodePath = config.getHeadRodNodePath(forceLeft);
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

        rodNode->setName((rodName + switchSuffix).data());
        rodNode->setTransform(leftTransform);
        snapNarrowSwitchRodParts(pointRails, rodNode, switchSuffix);
        rodNode->setParent(getMetaObject());
    };

    // 0. тяга привода
    {
        addRodNode(+RailwayMeshGeneratorConfig::SwitchRodIndex::RodHead, pointRailsLength, "RodHead");
    }
}

void
DynamicRailwayXJunction::snapNarrowSwitchRodParts(const NarrowPointRails& pointRails,
    const MetaUnigine::NodePtr& rodNode, const char* nameSuffixToAdd)
{
    std::unordered_map<std::string, std::function<Math::Vec3(bool left)>> snapPointGetters;

    auto getPointRailPoint =
        [&pointRails](int railIdx, const char* pointName, double defOffsetLength, bool left)
    {
        const auto& pointRail = pointRails[railIdx];
        double length = std::abs(pointRail->end.distanceOffset - pointRail->start.distanceOffset);
        const auto& nodeTransform = left ? pointRail->leftTransform : pointRail->rightTransform;
        if (pointRail->node)
        {
            auto rodNode = pointRail->node->findNode(pointName);
            if (rodNode)
            {
                return (nodeTransform * rodNode->getTransform()).getTranslate();
            }
        }
        return nodeTransform.getTranslate() + nodeTransform.getAxisX() * length * defOffsetLength;
    };

    snapPointGetters["leftWayLeftRailRod0"] = std::bind(getPointRailPoint,
        +NarrowPointRailIndex::LeftStock, "rod0", 1.0, std::placeholders::_1);
    snapPointGetters["leftWayRightRailRod0"] = std::bind(getPointRailPoint,
        +NarrowPointRailIndex::RightMiddle, "rod0", 1.0, std::placeholders::_1);
    snapPointGetters["rightWayLeftRailRod0"] = std::bind(getPointRailPoint,
        +NarrowPointRailIndex::LeftMiddle, "rod0", 1.0, std::placeholders::_1);
    snapPointGetters["rightWayRightRailRod0"] = std::bind(getPointRailPoint,
        +NarrowPointRailIndex::RightStock, "rod0", 1.0, std::placeholders::_1);

    auto getSnapPoint = [&snapPointGetters](const std::string& snapTo,
                            bool left) -> std::optional<Math::Vec3>
    {
        auto getterIt = snapPointGetters.find(snapTo);
        if (getterIt != snapPointGetters.end())
        {
            return getterIt->second(left);
        }
        return std::nullopt;
    };

    using SnapList = std::list<std::pair<Math::Vec3, std::string>>;
    std::list<std::pair<MetaUnigine::NodePtr, SnapList>> nodesToSnap;
    std::list<MetaUnigine::NodePtr> defferedNodes;

    for (int partIndex = 0; partIndex < rodNode->getNumChildren(); ++partIndex)
    {
        SnapList snaps;

        const auto& part = rodNode->getChild(partIndex);
        for (int childIndex = 0; childIndex < part->getNumChildren(); ++childIndex)
        {
            const auto& childNode = part->getChild(childIndex);

            if (auto snapTo = strstr(childNode->getName(), SnapToPrefix))
            {
                snapTo += strlen(SnapToPrefix);
                snaps.emplace_back(childNode->getTransform().getTranslate(), snapTo);
            }

            if (snaps.size() >= 2)
            {
                break;
            }
        }

        if (snaps.size() == 2)
        {
            nodesToSnap.emplace_back(part, std::move(snaps));
        }
    }

    while (nodesToSnap.size())
    {
        size_t sizeBeforePass = nodesToSnap.size();
        for (auto nodeToSnapIt = nodesToSnap.begin(); nodeToSnapIt != nodesToSnap.end();)
        {
            const auto& node = nodeToSnapIt->first;
            const auto& snaps = nodeToSnapIt->second;

            auto leftSnapPosFront = getSnapPoint(snaps.front().second, /*left=*/true);
            auto leftSnapPosBack = getSnapPoint(snaps.back().second, /*left=*/true);
            auto rightSnapPosFront = getSnapPoint(snaps.front().second, /*left=*/false);
            auto rightSnapPosBack = getSnapPoint(snaps.back().second, /*left=*/false);
            if (!leftSnapPosFront || !leftSnapPosBack || !rightSnapPosFront || !rightSnapPosBack)
            {
                ++nodeToSnapIt;
                continue;
            }

            auto leftTransform = transformFromTwoPointPairs(snaps.front().first, *leftSnapPosFront,
                snaps.back().first, *leftSnapPosBack);
            auto rightTransform = transformFromTwoPointPairs(snaps.front().first,
                *rightSnapPosFront, snaps.back().first, *rightSnapPosBack);

            auto component = MetaUnigine::getOrAddComponent<RailSwitchPartTransforms>(node);
            assert(component);
            component->setLeftTransform(leftTransform);
            component->setRightTransform(rightTransform);

            node->setParent(getMetaObject());
            node->setTransform(leftTransform);
            node->setName((std::string(node->getName()) + nameSuffixToAdd).data());

            for (int childIndex = 0; childIndex < node->getNumChildren(); ++childIndex)
            {
                const auto& childNode = node->getChild(childIndex);
                snapPointGetters[childNode->getName()] = [node, childNode](bool left)
                {
                    const auto& transfom = childNode->getTransform();

                    auto component = node->getComponent<RailSwitchPartTransforms>();
                    if (component)
                    {
                        if (left)
                        {
                            return (component->getLeftTransform() * transfom).getTranslate();
                        }
                        else
                        {
                            return (component->getRightTransform() * transfom).getTranslate();
                        }
                    }

                    return (node->getTransform() * transfom).getTranslate();
                };
            }

            nodeToSnapIt = nodesToSnap.erase(nodeToSnapIt);
        }
        if (nodesToSnap.size() == sizeBeforePass)
        {
            for (const auto& [partFailedToSnap, _] : nodesToSnap)
            {
                Log::error("Failed to snap \"%s\" part\n", partFailedToSnap->getName());
            }
            break;
        }
    }
}

std::optional<TransformSpline::NeighbourPoints>
DynamicRailwayXJunction::findMiddlePointRailsStart(SharedDataArgs& sharedData,
    const RailFrogInfo& frog)
{
    const double maxFrogOffsetM = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getPointRailsData().xNarrowSharpFrogMaxOffset);
    const double requiredOffset = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getPointRailsData().stockOffset + m_config->getPointRailsData().xMiddleGap);

    const auto& leftSpline = getSpline(sharedData, frog.leftInputSplineIdx);
    const auto& rightSpline = getSpline(sharedData, frog.rightInputSplineIdx);

    double leftStartDistance = frog.crossPoint.first.distanceOffset;
    double rightStartDistance = frog.crossPoint.second.distanceOffset;
    if (frog.isFlipped)
    {
        leftStartDistance -= maxFrogOffsetM;
        rightStartDistance -= maxFrogOffsetM;
    }

    TransformSpline frogMiddleSpline;
    frogMiddleSpline.build(
        leftSpline, rightSpline, /*k=*/[](double) { return 0.5; }, /*step=*/0.1, leftStartDistance,
        rightStartDistance, maxFrogOffsetM);

    auto middleSplineFrogCenter =
        frogMiddleSpline.findClosestPoint(toGlm(frog.transform.getTranslate()));
    if (!middleSplineFrogCenter)
    {
        return std::nullopt;
    }

    double stepM = 0.1;
    const double startDistance = middleSplineFrogCenter->distanceOffset;
    double endDistance = startDistance + maxFrogOffsetM;
    if (frog.isFlipped)
    {
        stepM = -stepM;
        endDistance = startDistance - maxFrogOffsetM;
        if (endDistance < 0.)
        {
            endDistance = 0.;
        }
    }

    const auto& leftSearch = getSplineSearch(sharedData, frog.leftInputSplineIdx);
    const auto& rightSearch = getSplineSearch(sharedData, frog.rightInputSplineIdx);
    return frogMiddleSpline.findPointsAcrossWithDistance(leftSearch, rightSearch, requiredOffset,
        stepM, startDistance, endDistance);
}

std::optional<TransformSpline::Point>
DynamicRailwayXJunction::findOppositeRailPoint(SharedDataArgs& sharedData, int closureSplineIdx,
    bool isLeft, const Math::Mat4& closureTransform)
{
    auto stockPoint = closureTransform.getTranslate() +
        closureTransform.getAxisY() * sharedData.railOffset * 2. * (isLeft ? -1. : 1);
    return getSplineSearch(sharedData, *getOppositeSplineId(sharedData, closureSplineIdx))
        .findClosestPoint(toGlm(stockPoint));
}

const DynamicRailwayXJunction::PointRailNearInfo*
DynamicRailwayXJunction::getNearPointRail(SharedDataArgs& sharedData, bool isFront, int splineIdx)
{
    auto pointRailNearIt = sharedData.pointRailsNearBySplineId.find(splineIdx);
    if (pointRailNearIt == sharedData.pointRailsNearBySplineId.end())
    {
        return nullptr;
    }
    const PointRailNearInfo* result = nullptr;
    for (const auto& pointRailInfo : pointRailNearIt->second)
    {
        if (!result ||
            (isFront && result->start > pointRailInfo.start) ||
            (!isFront && result->start < pointRailInfo.start))
        {
            result = &pointRailInfo;
        }
    }
    return result;
}

std::pair<std::optional<TransformSpline::Point>, std::optional<TransformSpline::Point>>
DynamicRailwayXJunction::generateWideSleepers(SharedDataArgs& sharedData, bool isFront,
    int splineIdx, const std::vector<int>& leftOuterSplinesIds,
    const std::vector<int>& rightOuterSplinesIds, int switchRodsType)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();

    const double sleeperLengthHalf =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getCommonData().railwayTrackWidth);
    const double sleeperWidth = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.sleeperWidth);
    const double sleeperStep = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.interval);
    const double sleeperMaxLengthHalf =
        RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.maxLength) / 2.0;

    const auto& middlePoint =
        isFront ? sharedData.firstSleeperTransform : sharedData.lastSleeperTransform;
    if (!middlePoint)
    {
        return {};
    }
    const auto glmLeft = toGlm(middlePoint->transform * Math::Vec3(0., sleeperMaxLengthHalf, 0.));
    const auto glmRight = toGlm(middlePoint->transform * Math::Vec3(0., -sleeperMaxLengthHalf, 0.));

    // Debug code:
    //if (m_debugVisualizer)
    //{
    //    m_debugVisualizer->addVector(fromGlm(glmLeft), fromGlm(glmRight));
    //}

    auto sleepersStartPoint =
        getSplineSearch(sharedData, splineIdx).findClosestPoints(glmLeft, glmRight);
    if (sleepersStartPoint == std::nullopt)
    {
        return {};
    }

    // Debug code:
    //if (m_debugVisualizer)
    //{
    //    m_debugVisualizer->addPoint(sleepersStartPoint->first.transform.getTranslate());
    //}

    auto pointRailNear = getNearPointRail(sharedData, isFront, splineIdx);
    if (pointRailNear == nullptr)
    {
        return {};
    }

    const auto tangent1 = sleepersStartPoint->first.transform.getAxisX();
    const auto tangent2 = sleepersStartPoint->second.transform.getAxisX();

    const double angle = std::abs(wrapValueAround(normalizedAngleFromToRad(tangent1, tangent2),
        -Math::Consts::PI05, Math::Consts::PI05));
    double distanceOffset = sleeperWidth * 2.;
    if (std::abs(angle - Math::Consts::PI05) > 0.01)
    {
        distanceOffset += sleeperLengthHalf / tan(angle);
    }
    double middleDst =
        sleepersStartPoint->first.distanceOffset +
        (isFront ? -distanceOffset : distanceOffset);
    if ((isFront && (middleDst < pointRailNear->start)) ||
        (!isFront && (middleDst > pointRailNear->start)))
    {
        // Reversed to single input, not two outs
        return {};
    }

    TransformSpline middleSpline;
    middleSpline.build(getSpline(sharedData, splineIdx), 0.5, middleDst, pointRailNear->start);

    auto bakFirstSleeperTransform = sharedData.firstSleeperTransform;
    sharedData.firstSleeperTransform.reset();
    auto bakLastSleeperTransform = sharedData.lastSleeperTransform;
    sharedData.lastSleeperTransform.reset();

    std::set<int> pathIds;
    for (const auto& splineId : leftOuterSplinesIds)
    {
        if (auto srcPathId = getSrcPathIdx(sharedData, splineId))
        {
            pathIds.emplace(*srcPathId);
        }
    }
    for (const auto& splineId : rightOuterSplinesIds)
    {
        if (auto srcPathId = getSrcPathIdx(sharedData, splineId))
        {
            pathIds.emplace(*srcPathId);
        }
    }
    auto addMiddleSleeperInfoFunc = [this, &sharedData, &pathIds](const Math::Mat4& middle)
    {
        auto worldTransform = getTransform() * middle;
        for (auto pathIdx : pathIds)
        {
            sharedData.sleepersInfos[pathIdx]->emplace_back(
                DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
        }
    };

    generateMiddleSleepers(sharedData, middleSpline,
        uniteVecs(leftOuterSplinesIds, rightOuterSplinesIds),
        [this, &sharedData, &middleSpline, &leftOuterSplinesIds, &rightOuterSplinesIds,
            addMiddleSleeperInfoFunc, switchRodsType, isFront](double startDst,
            double endDst,
            double step, int type, bool skipFirst, bool skipLast,
            const AddMountsFuncs& addMountsFuncs)
    {
        doAddMiddleSleepersSwitchBoxOverride(sharedData, middleSpline, startDst, endDst, step, type,
            skipFirst, skipLast, leftOuterSplinesIds, rightOuterSplinesIds, switchRodsType, !isFront,
            addMountsFuncs, addMiddleSleeperInfoFunc);
    });

    auto result = std::make_pair(sharedData.firstSleeperTransform, sharedData.lastSleeperTransform);
    sharedData.firstSleeperTransform = bakFirstSleeperTransform;
    sharedData.lastSleeperTransform = bakLastSleeperTransform;
    return result;
}

void
DynamicRailwayXJunction::generateSleepersOnXOut(SharedDataArgs& sharedData, int splineIdx,
    int otherSplineIdx, const Unigine::Math::Mat4& lastWideSleeperTransform, double endDistance)
{
    auto addSleeperInfoFunc = [this, &sharedData, splineIdx, otherSplineIdx](
                                  const Math::Mat4& transform)
    {
        auto worldTransform = getTransform() * transform;

        sharedData.sleepersInfos[otherSplineIdx]->emplace_back(
            DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
        sharedData.sleepersInfos[splineIdx]->emplace_back(
            DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
    };
    generateSleepersOnOut(sharedData, splineIdx, lastWideSleeperTransform,
        endDistance, addSleeperInfoFunc);
}

void
DynamicRailwayXJunction::generateSleepersOnOuts(SharedDataArgs& sharedData, bool isLeftNarrow,
    bool isRightNarrow)
{
    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);

    if (!isLeftNarrow)
    {
        auto firstLastTransformPair = generateWideSleepers(sharedData, /*isFront=*/true,
            +InternalSplineType::RailStockLeft,
            {+InternalSplineType::RailStockLeft, +InternalSplineType::RailClosureBackRight},
            {+InternalSplineType::RailClosureFrontLeft, +InternalSplineType::RailMiddleLeftToLeft},
            +RailwayMeshGeneratorConfig::SwitchRodsType::XBackRight);
        if (firstLastTransformPair.first)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathLeftToRight,
                +InternalSplineType::PathLeftToLeft, firstLastTransformPair.first->transform,
                sleeperStep / 2.);
        }

        firstLastTransformPair = generateWideSleepers(sharedData, /*isFront=*/false,
            +InternalSplineType::RailStockLeft,
            {+InternalSplineType::RailStockLeft, +InternalSplineType::RailClosureFrontRight},
            {+InternalSplineType::RailClosureBackLeft, +InternalSplineType::RailMiddleLeftToLeft},
            +RailwayMeshGeneratorConfig::SwitchRodsType::XFrontRight);
        if (firstLastTransformPair.second)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathRightToLeft,
                +InternalSplineType::PathLeftToLeft, firstLastTransformPair.second->transform,
                -sleeperStep / 2.);
        }
    }
    else
    {
        if (sharedData.firstSleeperTransform)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathRightToLeft,
                +InternalSplineType::PathLeftToLeft, sharedData.firstSleeperTransform->transform,
                sleeperStep / 2.);
        }
        else
        {
            Log::error("sharedData.firstSleeperTransform == std::nullopt for %d junction\n",
                m_key.id);
        }

        if (sharedData.lastSleeperTransform)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathRightToLeft,
                +InternalSplineType::PathLeftToLeft, sharedData.lastSleeperTransform->transform,
                -sleeperStep / 2.);
        }
        else
        {
            Log::error("sharedData.lastSleeperTransform == std::nullopt for %d junction\n",
                m_key.id);
        }
    }

    if (!isRightNarrow)
    {
        auto firstLastTransformPair =
            generateWideSleepers(sharedData, /*isFront=*/true, +InternalSplineType::RailStockRight,
            {+InternalSplineType::RailClosureFrontRight,
                +InternalSplineType::RailMiddleRightToRight},
            {+InternalSplineType::RailStockRight, +InternalSplineType::RailClosureBackLeft},
            +RailwayMeshGeneratorConfig::SwitchRodsType::XBackLeft);
        if (firstLastTransformPair.first)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathRightToLeft,
                +InternalSplineType::PathRightToRight, firstLastTransformPair.first->transform,
                sleeperStep / 2.);
        }

        firstLastTransformPair =
            generateWideSleepers(sharedData, /*isFront=*/false, +InternalSplineType::RailStockRight,
            {+InternalSplineType::RailClosureBackRight,
                +InternalSplineType::RailMiddleRightToRight},
            {+InternalSplineType::RailStockRight, +InternalSplineType::RailClosureFrontLeft},
            +RailwayMeshGeneratorConfig::SwitchRodsType::XFrontLeft);
        if (firstLastTransformPair.second)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathLeftToRight,
                +InternalSplineType::PathRightToRight, firstLastTransformPair.second->transform,
                -sleeperStep / 2.);
        }
    }
    else
    {
        if (sharedData.firstSleeperTransform)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathRightToLeft,
                +InternalSplineType::PathRightToRight, sharedData.firstSleeperTransform->transform,
                sleeperStep / 2.);
        }
        else
        {
            Log::error("sharedData.firstSleeperTransform == std::nullopt for %d junction\n",
                m_key.id);
        }

        if (sharedData.lastSleeperTransform)
        {
            generateSleepersOnXOut(sharedData, +InternalSplineType::PathLeftToRight,
                +InternalSplineType::PathRightToRight, sharedData.lastSleeperTransform->transform,
                -sleeperStep / 2.);
        }
        else
        {
            Log::error("sharedData.lastSleeperTransform == std::nullopt for %d junction\n",
                m_key.id);
        }
    }
}

void
DynamicRailwayXJunction::addJunctionProperty(SharedDataArgs& sharedData)
{
    message("adding juncton property component...\n");

    int propIndex = getMetaObject()->findProperty("junction");
    if (propIndex < 0)
    {
        propIndex = getMetaObject()->addProperty("junction");

        PropertyPtr props = getMetaObject()->getProperty(propIndex);
        PropertyParameterPtr typeParam = props->getParameterPtr("type");
        typeParam->setValueSwitch(JUNCTION_TYPE_X);
    }
}

void
DynamicRailwayXJunction::addConnectionPoints(SharedDataArgs& sharedData)
{
    message("adding connection points...\n");

    const int leftPathIdx = +InternalSplineType::PathLeftToRight;
    const int rightPathIdx = +InternalSplineType::PathLeftToLeft;
    const int totalPaths = 2;

    const int JUNCTION_POINT_CENTRAL = 0;
    const int JUNCTION_POINT_LEFT_INPUT = 1;
    const int JUNCTION_POINT_RIGHT_INPUT = 2;
    const int JUNCTION_POINT_LEFT_OUTPUT = 3;
    const int JUNCTION_POINT_RIGHT_OUTPUT = 4;
    const int JUNCTION_POINT_TOTAL = 5;

    Math::Vec3 points[JUNCTION_POINT_TOTAL];

    const auto& leftToRightSegments =
        getSpline(sharedData, +InternalSplineType::PathLeftToRight).getSegmentsMap();
    points[JUNCTION_POINT_LEFT_INPUT] = fromGlm(leftToRightSegments(0)->p0());
    points[JUNCTION_POINT_RIGHT_OUTPUT] =
        fromGlm(leftToRightSegments(leftToRightSegments.iterative_size() - 1)->p3());

    const auto& rightToLeftSegments =
        getSpline(sharedData, +InternalSplineType::PathRightToLeft).getSegmentsMap();
    points[JUNCTION_POINT_RIGHT_INPUT] = fromGlm(rightToLeftSegments(0)->p0());
    points[JUNCTION_POINT_LEFT_OUTPUT] =
        fromGlm(rightToLeftSegments(rightToLeftSegments.iterative_size() - 1)->p3());

    points[JUNCTION_POINT_CENTRAL] = sharedData.middleCrossPoint.transform.getTranslate();

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
DynamicRailwayXJunction::isLeftNarrowJunction(SharedDataArgs& sharedData,
    const Math::Mat4& leftFrogTransform, const Math::Mat4& rightFrogTransform)
{
    if (sharedData.splineById[+InternalSplineType::RailMiddleRightToRight].empty())
    {
        return true;
    }

    const auto& middleSearch =
        getSplineSearch(sharedData, +InternalSplineType::RailMiddleRightToRight);

    auto glmLeftPoint = toGlm(leftFrogTransform.getTranslate());
    auto leftMiddleNearest = middleSearch.findClosestPoint(glmLeftPoint);
    if (!leftMiddleNearest)
    {
        throw std::logic_error("Failed to measure distance from left frog to left middle rail");
        return false;
    }
    double leftDistance2 =
        (leftMiddleNearest->transform.getTranslate() - leftFrogTransform.getTranslate()).length2();

    auto glmRightPoint = toGlm(rightFrogTransform.getTranslate());
    auto rightMiddleNearest = middleSearch.findClosestPoint(glmRightPoint);
    if (!rightMiddleNearest)
    {
        throw std::logic_error("Failed to measure distance from right frog to left middle rail");
        return false;
    }
    double rightDistance2 =
        (rightMiddleNearest->transform.getTranslate() - rightFrogTransform.getTranslate())
            .length2();

    return (leftDistance2 < rightDistance2);
}

bool
DynamicRailwayXJunction::isRightNarrowJunction(SharedDataArgs& sharedData,
    const Math::Mat4& leftFrogTransform, const Math::Mat4& rightFrogTransform)
{
    if (sharedData.splineById[+InternalSplineType::RailMiddleLeftToLeft].empty())
    {
        return true;
    }

    const auto& middleSearch =
        getSplineSearch(sharedData, +InternalSplineType::RailMiddleLeftToLeft);

    auto glmLeftPoint = toGlm(leftFrogTransform.getTranslate());
    auto leftMiddleNearest = middleSearch.findClosestPoint(glmLeftPoint);
    if (!leftMiddleNearest)
    {
        throw std::logic_error("Failed to measure distance from left frog to right middle rail");
        return false;
    }
    double leftDistance2 =
        (leftMiddleNearest->transform.getTranslate() - leftFrogTransform.getTranslate()).length2();

    auto glmRightPoint = toGlm(rightFrogTransform.getTranslate());
    auto rightMiddleNearest = middleSearch.findClosestPoint(glmRightPoint);
    if (!rightMiddleNearest)
    {
        throw std::logic_error("Failed to measure distance from right frog to right middle rail");
        return false;
    }
    double rightDistance2 =
        (rightMiddleNearest->transform.getTranslate() - rightFrogTransform.getTranslate())
            .length2();

    return (leftDistance2 > rightDistance2);
}
