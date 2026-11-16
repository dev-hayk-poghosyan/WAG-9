#include "DynamicRailwayXCross.h"

#include "neokon-shared/GlmUnigineInterop.h"
#include "neokon-shared/metaunigine/MetaUnigine.h"
#include "neokon-shared/NormalizedAngleFromTo.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConstants.h"
#include "neokon-shared/railways/RailSwitchInfoComponents.h"
#include "neokon-shared/railways/DynamicRailwayBranchSegment.h"

using namespace Unigine;

DynamicRailwayXCross::DynamicRailwayXCross(uint32_t partId, const std::string& configPath,
    std::vector<BezierSpline>&& splines,
    std::vector<SplineNeighborConfigs>&& neighborConfigs,
    const Unigine::Math::Mat4& transform)
    : DynamicRailwayJunctionBase(partId, configPath, std::move(splines),
        std::move(neighborConfigs), transform)
{
}

bool DynamicRailwayXCross::load()
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

    //right closure with left stock - top obtuse
    auto topObtuseFrog = generateRailFrog(sharedData, +InternalSplineType::PathRightToLeft_LeftRail,
        +InternalSplineType::PathLeftToRight_LeftRail, /*flip=*/true, /*isObtuse=*/true);
    if (!topObtuseFrog)
    {
        return false;
    }

    if (isDynamicRailwayPartTypeEnabled(DynamicRailwayPartType::JunctionRails))
    {
        // right closure with left closure - front sharp
        auto frog = generateRailFrog(sharedData, +InternalSplineType::PathLeftToRight_RightRail,
            +InternalSplineType::PathRightToLeft_LeftRail, /*flip=*/false, /*isObtuse=*/false);
        if (frog)
        {
            generateGuardRailsForRailFrog(sharedData, *frog, sharedData.guardRailClosureOffset);
        }

        // right stock with left left stock - back sharp
        frog = generateRailFrog(sharedData, +InternalSplineType::PathRightToLeft_RightRail,
            +InternalSplineType::PathLeftToRight_LeftRail, /*flip=*/true, /*isObtuse=*/false);
        if (frog)
        {
            generateGuardRailsForRailFrog(sharedData, *frog, sharedData.guardRailClosureOffset);
        }

        // right stock with left closure - bottom obtuse
        generateRailFrog(sharedData, +InternalSplineType::PathRightToLeft_RightRail,
            +InternalSplineType::PathLeftToRight_RightRail, /*flip=*/false, /*isObtuse=*/true);

        generateRails(sharedData);
    }

    for (int pathIdx = 0; pathIdx < 2; ++pathIdx)
    {
        sharedData.sleepersInfos.emplace_back(new DynamicRailwayPartWithSleepers::Sleepers);
    }

    if (topObtuseFrog->angle > 60.0)
    {
        generateSingleSleepers(sharedData, topObtuseFrog->angle);
    }
    else
    {
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
            interFillVec(+InternalSplineType::PathLeftToRight_LeftRail,
            +InternalSplineType::PathRightToLeft_RightRail),
            addMiddleSleeperInfoFunc);
        generateSleepersOnOuts(sharedData);
    }
    setSleepers(std::move(sharedData.sleepersInfos));

    mesh->createBounds();
    getMetaObject()->setMesh(mesh);

    addJunctionProperty(sharedData);
    addConnectionPoints(sharedData);
    setupMeshMaterials(sharedData);

    return true;
}

bool DynamicRailwayXCross::isDifferentWith(const DynamicRailwayPartPtr& old) const
{
    auto part = std::dynamic_pointer_cast<DynamicRailwayXCross>(old);
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

//------------------------------------------------------------------------------------------
// Generation steps
//------------------------------------------------------------------------------------------

std::optional<TransformSpline::NeighbourPoints>
DynamicRailwayXCross::validateInputSplinesAndFindCrossPoint(SharedDataArgs& sharedData)
{
    // Check if input splines are swapped, fix if required
    {
        const auto& leftPathSpline = getSpline(sharedData, +InternalSplineType::PathLeftToRight);
        const auto& rightPathSpline = getSpline(sharedData, +InternalSplineType::PathRightToLeft);

        auto leftStart = leftPathSpline.pointAtDistance(0.0);
        auto leftEnd = leftPathSpline.pointAtDistance(leftPathSpline.arcLength());
        auto rightStart = rightPathSpline.pointAtDistance(0.0);
        auto rightEnd = rightPathSpline.pointAtDistance(rightPathSpline.arcLength());
        if (!leftStart || !leftEnd || !rightStart || !rightEnd)
        {
            return std::nullopt;
        }

        auto leftDir = Math::normalize(fromGlm(*leftEnd) - fromGlm(*leftStart));
        auto rightDir = Math::normalize(fromGlm(*rightEnd) - fromGlm(*rightStart));
        if (Math::cross(leftDir, rightDir).z < 0.0)
        {
            warning("Swapped left/right splines detected, fix\n");
            swapSplines(+InternalSplineType::PathLeftToRight, +InternalSplineType::PathRightToLeft);
        }
    }

    const auto& leftPathSpline = getSpline(sharedData, +InternalSplineType::PathLeftToRight);
    const auto& rightPathSpline = getSpline(sharedData, +InternalSplineType::PathRightToLeft);

    auto& leftPathSearch = sharedData.splineSearchById[+InternalSplineType::PathLeftToRight];
    leftPathSearch.build(leftPathSpline);

    // find paths cross point
    std::optional<TransformSpline::NeighbourPoints> crossPoint =
        leftPathSearch.findPointsWithDistance(rightPathSpline, 0.0);
    if (!crossPoint)
    {
        error("Unable to find preliminary closure cross point\n");
        return std::nullopt;
    }

    const auto& firstPoint = crossPoint->first.transform.getTranslate();
    const auto& secondPoint = crossPoint->second.transform.getTranslate();
    if ((firstPoint.xy - secondPoint.xy).length() > 0.1)
    {
        error("Preliminary cross points too far\n");
        return std::nullopt;
    }

    if (firstPoint.z - secondPoint.z > 0.05)
    {
        const auto& tangent1 = crossPoint->first.transform.getAxisX();
        const auto& tangent2 = crossPoint->second.transform.getAxisX();

        const double angle = std::abs(normalizedAngleFromToRad(tangent1, tangent2));
        const double distanceOffset = sharedData.trackWidth * 0.5 / tan(angle / 2.);

        double middleZ = (firstPoint.z + secondPoint.z) / 2.;
        auto fixPointFunc = [middleZ](double distance, glm::dvec3& position,
                                glm::dvec3& backTangent, glm::dvec3& frontTangent)
        {
            position.z = middleZ;
            backTangent.z = middleZ;
            frontTangent.z = middleZ;
        };

        auto fixSpline = [this, &sharedData, &fixPointFunc, &distanceOffset](int splineId,
                double crossDistance, const Math::dvec3& crossPoint, const Math::dvec3& tangent)
        {
            auto spline = getSpline(sharedData, splineId);
            BezierSpline newSpline(spline);

            std::vector<double> pointsToInsert;
            pointsToInsert.emplace_back(crossDistance);
            pointsToInsert.emplace_back(crossDistance - distanceOffset);
            pointsToInsert.emplace_back(crossDistance + distanceOffset);
            newSpline.insertPoints(pointsToInsert, fixPointFunc);

            setSpline(sharedData, splineId, std::move(newSpline));
        };

        fixSpline(+InternalSplineType::PathLeftToRight, crossPoint->first.distanceOffset,
            firstPoint,
            tangent1);
        fixSpline(+InternalSplineType::PathRightToLeft, crossPoint->second.distanceOffset,
            secondPoint,
            tangent2);

        leftPathSearch.build(getSpline(sharedData, +InternalSplineType::PathLeftToRight));
        crossPoint = leftPathSearch.findPointsWithDistance(
            getSpline(sharedData, +InternalSplineType::PathRightToLeft), 0.0);
    }

    return crossPoint;
}

bool DynamicRailwayXCross::buildSplines(SharedDataArgs& sharedData)
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

    sharedData.trackWidth = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getCommonData().railwayTrackWidth);
    const double profileWidth = maxProfileY - minProfileY;
    const double halfProfileWidth = profileWidth * 0.5;
    const double offset = (sharedData.trackWidth + halfProfileWidth) * 0.5;
    const double sleeperWidth = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getJunctionSleepersData().minLength);
    const double halfSleeperWidth = sleeperWidth * 0.5;
    const double guardRailStockOffset = RailwayMeshGeneratorConfig::toInnerUnits(
        m_config->getGuardRailsData().offset);

    sharedData.guardRailClosureOffset = offset * 2. - guardRailStockOffset;

    // Validate cross point distances
    auto crossPoint = validateInputSplinesAndFindCrossPoint(sharedData);
    if (!crossPoint || crossPoint->gap > 0.1)
    {
        error("Unable to find closure cross point\n");
        return false;
    }

    const auto& leftPathSpline = getSpline(sharedData, +InternalSplineType::PathLeftToRight);
    const auto& rightPathSpline = getSpline(sharedData, +InternalSplineType::PathRightToLeft);

    // paths for sleepers on outs, PathLeftToRight was built while validation - skip here
    //sharedData.splineSearchById[+InternalSplineType::PathLeftToRight].build(
    //    getSpline(sharedData, +InternalSplineType::PathLeftToRight));
    sharedData.splineSearchById[+InternalSplineType::PathRightToLeft].build(
        getSpline(sharedData, +InternalSplineType::PathRightToLeft));

    //left path  - left rail
    sharedData.railInfoById[+InternalSplineType::PathLeftToRight_LeftRail] = {
        "PathLeftToRight_LeftRail", false};
    sharedData.splineById[+InternalSplineType::PathLeftToRight_LeftRail]
        = leftPathSpline.createOffseted(glm::dvec3(0., offset, 0.));
    sharedData.splineSearchById[+InternalSplineType::PathLeftToRight_LeftRail]
        .build(sharedData.splineById[+InternalSplineType::PathLeftToRight_LeftRail]);

    //left path  - right rail
    sharedData.railInfoById[+InternalSplineType::PathLeftToRight_RightRail] = {
        "PathLeftToRight_RightRail", true};
    sharedData.splineById[+InternalSplineType::PathLeftToRight_RightRail]
        = leftPathSpline.createOffseted(glm::dvec3(0., -offset, 0.));
    sharedData.splineSearchById[+InternalSplineType::PathLeftToRight_RightRail]
        .build(sharedData.splineById[+InternalSplineType::PathLeftToRight_RightRail]);

    //right path  - left rail
    sharedData.railInfoById[+InternalSplineType::PathRightToLeft_LeftRail] = {
        "PathRightToLeft_LeftRail", false};
    sharedData.splineById[+InternalSplineType::PathRightToLeft_LeftRail]
        = rightPathSpline.createOffseted(glm::dvec3(0., offset, 0.));
    sharedData.splineSearchById[+InternalSplineType::PathRightToLeft_LeftRail]
        .build(sharedData.splineById[+InternalSplineType::PathRightToLeft_LeftRail]);

    //right path  - right rail
    sharedData.railInfoById[+InternalSplineType::PathRightToLeft_RightRail] = {
        "PathRightToLeft_RightRail", true};
    sharedData.splineById[+InternalSplineType::PathRightToLeft_RightRail]
        = rightPathSpline.createOffseted(glm::dvec3(0., -offset, 0.));
    sharedData.splineSearchById[+InternalSplineType::PathRightToLeft_RightRail]
        .build(sharedData.splineById[+InternalSplineType::PathRightToLeft_RightRail]);

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

    double middleCrossError =
        (middleCrossPoint->transform.getTranslate() - closureCrossPointPosition).length();
    if (middleCrossError > 0.2)
    {
        warning("Too big distance between sleepers and closure spline's cross points == %f\n",
            middleCrossError);
    }

    sharedData.middleCrossPoint = *middleCrossPoint;

    sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::PathLeftToRight_LeftRail);
    sharedData.leftOuterSplinesIds.push_back(+InternalSplineType::PathRightToLeft_LeftRail);
    sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::PathRightToLeft_RightRail);
    sharedData.rightOuterSplinesIds.push_back(+InternalSplineType::PathLeftToRight_RightRail);

    return true;
}

void
DynamicRailwayXCross::generateSleepersOnOuts(SharedDataArgs& sharedData)
{
    const double sleeperStep =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getJunctionSleepersData().interval);
    if (sharedData.firstSleeperTransform)
    {
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathLeftToRight]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };

            generateSleepersOnOut(sharedData, +InternalSplineType::PathLeftToRight,
                sharedData.firstSleeperTransform->transform, sleeperStep / 2., addSleeperInfoFunc);
        }
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathRightToLeft]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };

            generateSleepersOnOut(sharedData, +InternalSplineType::PathRightToLeft,
                sharedData.firstSleeperTransform->transform, sleeperStep / 2., addSleeperInfoFunc);
        }
    }
    else
    {
        Log::error("sharedData.firstSleeperTransform == std::nullopt for %d junction\n", m_key.id);
    }

    if (sharedData.lastSleeperTransform)
    {
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathLeftToRight]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };

            generateSleepersOnOut(sharedData, +InternalSplineType::PathLeftToRight,
                sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc);
        }
        {
            auto addSleeperInfoFunc = [this, &sharedData](const Math::Mat4& transform)
            {
                auto worldTransform = getTransform() * transform;
                sharedData.sleepersInfos[+InternalSplineType::PathRightToLeft]->emplace_back(
                    DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
            };

            generateSleepersOnOut(sharedData, +InternalSplineType::PathRightToLeft,
                sharedData.lastSleeperTransform->transform, -sleeperStep / 2., addSleeperInfoFunc);
        }
    }
    else
    {
        Log::error("sharedData.lastSleeperTransform == std::nullopt for %d junction\n", m_key.id);
    }
}

void
DynamicRailwayXCross::generateSingleSleepers(SharedDataArgs& sharedData, double angleDeg)
{
    const auto& sleepersData = m_config->getJunctionSleepersData();

    const double sleeperLengthHalf =
        RailwayMeshGeneratorConfig::toInnerUnits(m_config->getCommonData().railwayTrackWidth);
    const double sleeperWidth = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.sleeperWidth);
    const double sleeperStep = RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.interval);
    const double sleeperMaxLengthHalf =
        RailwayMeshGeneratorConfig::toInnerUnits(sleepersData.maxLength) / 2.0;

    const double angleRad = angleDeg * Math::Consts::DEG2RAD_D;

    double distanceOffset = sleeperWidth * 4.;
    if (std::abs(angleRad - Math::Consts::PI05) < 0.01)
    {
        distanceOffset += sleeperLengthHalf / tan(angleRad);
    }

    //

    const auto& leftToRight = getSplineSearch(sharedData, +InternalSplineType::PathLeftToRight);
    const auto& leftToRightLeftRail =
        getSplineSearch(sharedData, +InternalSplineType::PathLeftToRight_LeftRail);
    const auto& leftToRightRightRail =
        getSplineSearch(sharedData, +InternalSplineType::PathLeftToRight_RightRail);

    const auto& rightToLeft = getSplineSearch(sharedData, +InternalSplineType::PathRightToLeft);
    const auto& rightToLeftLeftRail =
        getSplineSearch(sharedData, +InternalSplineType::PathRightToLeft_LeftRail);
    const auto& rightToLeftRightRail =
        getSplineSearch(sharedData, +InternalSplineType::PathRightToLeft_RightRail);

    auto leftToRightCross1 = leftToRight.findClosestPoints(rightToLeftLeftRail);
    auto leftToRightCross2 = leftToRight.findClosestPoints(rightToLeftRightRail);
    if (!leftToRightCross1 || !leftToRightCross2)
    {
        return;
    }

    double leftToRightCrossMin =
        std::min(leftToRightCross1->first.distanceOffset, leftToRightCross2->first.distanceOffset) -
        distanceOffset;
    double leftToRightCrossMax =
        std::max(leftToRightCross1->first.distanceOffset, leftToRightCross2->first.distanceOffset) +
        distanceOffset;

    auto rightToLeftCross1 = rightToLeft.findClosestPoints(leftToRightLeftRail);
    auto rightToLeftCross2 = rightToLeft.findClosestPoints(leftToRightRightRail);
    if (!rightToLeftCross1 || !rightToLeftCross2)
    {
        return;
    }

    double rightToLeftCrossMin =
        std::min(rightToLeftCross1->first.distanceOffset, rightToLeftCross2->first.distanceOffset) -
        distanceOffset;
    double rightToLeftCrossMax =
        std::max(rightToLeftCross1->first.distanceOffset, rightToLeftCross2->first.distanceOffset) +
        distanceOffset;

    //

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

    auto addSleeper =
        [this, &getRandomSleeperCluster, &sharedData](int pathIdx, const Math::Mat4& transform)
    {
        auto cluster = getRandomSleeperCluster();
        cluster->transforms.push_back(getTransform() * transform);

        auto worldTransform = getTransform() * transform;
        sharedData.sleepersInfos[pathIdx]->emplace_back(
            DynamicRailwayPartWithSleepers::Sleeper{worldTransform});
    };

    auto calcSleepersTransforms =
        [&addSleeper](int pathIdx, const BezierSpline& spline, double startM, double endM, double stepM)
    {
        double fullDistance = endM - startM;
        double numSleeperGaps = std::ceil(fullDistance / stepM);
        if (numSleeperGaps > 0.)
        {
            stepM = fullDistance / numSleeperGaps;
        }

        double distance = startM;
        int numSleepersInt(numSleeperGaps + 1.);
        for (int sleeperIdx = 0; sleeperIdx < numSleepersInt; ++sleeperIdx)
        {
            if (auto sleeperTransform = spline.calcSplineMatrixAtDistance(distance))
            {
                addSleeper(pathIdx, *sleeperTransform);
            }
            distance += stepM;
        }
    };

    const auto& leftToRightSpline = getSpline(sharedData, +InternalSplineType::PathLeftToRight);
    const double leftToRightMaxDistance = leftToRightSpline.arcLength() - sleeperStep / 2.;
    calcSleepersTransforms(+InternalSplineType::PathLeftToRight, leftToRightSpline, sleeperStep / 2.,
        leftToRightCrossMin, sleeperStep);

    calcSleepersTransforms(+InternalSplineType::PathLeftToRight, leftToRightSpline,
        leftToRightCrossMin + sleeperStep,
        leftToRightCrossMin + sleeperStep, sleeperStep);
    calcSleepersTransforms(+InternalSplineType::PathLeftToRight, leftToRightSpline,
        leftToRightCrossMin + sleeperStep * 3.,
        leftToRightCrossMax - sleeperStep * 3., sleeperStep);
    calcSleepersTransforms(+InternalSplineType::PathLeftToRight, leftToRightSpline,
        leftToRightCrossMax - sleeperStep,
        leftToRightCrossMax - sleeperStep, sleeperStep);
    calcSleepersTransforms(+InternalSplineType::PathLeftToRight, leftToRightSpline,
        leftToRightCrossMax, leftToRightMaxDistance,
        sleeperStep);

    const auto& rightToLeftSpline = getSpline(sharedData, +InternalSplineType::PathRightToLeft);
    const double rightToLeftMaxDistance = rightToLeftSpline.arcLength() - sleeperStep / 2.;
    calcSleepersTransforms(+InternalSplineType::PathRightToLeft, rightToLeftSpline,
        sleeperStep / 2., rightToLeftCrossMin, sleeperStep);
    calcSleepersTransforms(+InternalSplineType::PathRightToLeft, rightToLeftSpline,
        rightToLeftCrossMax, rightToLeftMaxDistance, sleeperStep);
}

void DynamicRailwayXCross::addJunctionProperty(SharedDataArgs& sharedData)
{
    message("adding juncton property component...\n");

    int propIndex = getMetaObject()->findProperty("junction");
    if (propIndex < 0)
    {
        propIndex = getMetaObject()->addProperty("junction");

        PropertyPtr props = getMetaObject()->getProperty(propIndex);
        PropertyParameterPtr typeParam = props->getParameterPtr("type");
        typeParam->setValueSwitch(JUNCTION_TYPE_CROSS);
    }
}

void DynamicRailwayXCross::addConnectionPoints(SharedDataArgs& sharedData)
{
    message("adding connection points...\n");

    const int leftPathIdx = 0;
    const int rightPathIdx = 1;
    const int totalPaths = 2;

    const int JUNCTION_POINT_CENTRAL = 0;
    const int JUNCTION_POINT_LEFT_INPUT = 1;
    const int JUNCTION_POINT_RIGHT_INPUT = 2;
    const int JUNCTION_POINT_LEFT_OUTPUT = 3;
    const int JUNCTION_POINT_RIGHT_OUTPUT = 4;
    const int JUNCTION_POINT_TOTAL = 5;

    Math::Vec3 points[JUNCTION_POINT_TOTAL];

    for (int pathIdx = 0; pathIdx < totalPaths; ++pathIdx)
    {
        if (pathIdx == leftPathIdx)
        {
            const auto& segments = getSplines()[leftPathIdx].getSegmentsMap();
            points[JUNCTION_POINT_LEFT_INPUT] = fromGlm(segments(0)->p0());
            points[JUNCTION_POINT_RIGHT_OUTPUT] =
                fromGlm(segments(segments.iterative_size() - 1)->p3());
        }
        else if (pathIdx == rightPathIdx)
        {
            const auto& segments = getSplines()[rightPathIdx].getSegmentsMap();
            points[JUNCTION_POINT_RIGHT_INPUT] = fromGlm(segments(0)->p0());
            points[JUNCTION_POINT_LEFT_OUTPUT] =
                fromGlm(segments(segments.iterative_size() - 1)->p3());
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
