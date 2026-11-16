#pragma once

#include "neokon-shared/GenerateMeshFromSplineList.h"

#include "neokon-shared/TransformSpline.h"
#include "neokon-shared/railways/DynamicRailwayJunctionBase.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"

#include <UnigineMathLib.h>
#include <UnigineMesh.h>
#include <UnigineObjects.h>

class DynamicRailwayThreeWayJunction;
using DynamicRailwayThreeWayJunctionPtr = std::shared_ptr<DynamicRailwayThreeWayJunction>;

/**
 * This class is used by 'DynamicRailways' to load/unload and create nearby railway Y junctions
 * in runtime. @see documentation to base class DynamicRailwayPart.
 */
class DynamicRailwayThreeWayJunction : public DynamicRailwayJunctionBase
{
public:
    static const char* NearSwitchSuffix;
    static const char* MiddleSwitchSuffix;

    /**
     * Типы рабочих сплайнов по которым происходит построение стрелки.
     */
    enum class InternalSplineType
    {
        PathRootToSide = 0,
        PathRootToMiddle,
        PathMiddleToLeft,
        PathMiddleToRight,

        RailToSideLeft,
        RailToSideRight,
        RailToMiddleLeft,
        RailToMiddleRight,
        RailToLeftLeft,
        RailToLeftRight,
        RailToRightLeft,
        RailToRightRight,

        Total
    };

    DynamicRailwayThreeWayJunction(uint32_t partId, const std::string& configPath,
        std::vector<BezierSpline>&& splines,
        std::vector<SplineNeighborConfigs>&& neighborConfigs,
        const Unigine::Math::Mat4& transform);

    virtual bool load() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual Unigine::NodePtr createNodes() override;

    bool isLeftSide() const { return m_isLeftSide; }
    void switchToSide();
    void switchToLeft();
    void switchToRight();

private:
    enum class PointRailIndex
    {
        NearLeft = 0,
        NearRight,
        FarLeft,
        FarRight,
        Total
    };

    struct SharedDataArgs : SharedDataArgsBase
    {
        TransformSpline middleSpline;

        bool isLeftSide{false};
        bool isRightSide{false};

        double railOffset{0.0};
        double railProfileWidth{0.0};
        double guardRailClosureOffset{ 0.0 };
        double pointRailStockOffset{0.0};

        std::vector<DynamicRailwayPartWithSleepers::SleepersPtr> sleepersInfos;
    };
    void validateMiddleSplines(SharedDataArgs& sharedData);
    bool buildSplines(SharedDataArgs& sharedData);

    std::optional<TransformSpline::NeighbourPoints> findPointRailRoot(
        SharedDataArgs& sharedData, double offset, int leftSplineIdx, int rightSplineIdx,
        double startM, double endM);

    void generateMiddlePointRailsForLeftSide(SharedDataArgs& sharedData,
        const RailFrogInfo& firstSideFrog);
    void generateRootPointRailsForLeftSide(SharedDataArgs& sharedData,
        const RailFrogInfo& firstSideFrog);

    void generateMiddlePointRailsForRightSide(SharedDataArgs& sharedData,
        const RailFrogInfo& firstSideFrog);
    void generateRootPointRailsForRightSide(SharedDataArgs& sharedData,
        const RailFrogInfo& firstSideFrog);

    void generateCommonSwitchBox(const Unigine::Math::Mat4& transform,
        RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType);
    void generateSwitchRods(SharedDataArgs& sharedData, const PointRailInfo& leftPointRail,
        const PointRailInfo& rightPointRail, const char* switchSuffix,
        RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType);

    void generateSleepersOnOuts(SharedDataArgs& sharedData);
    void addJunctionProperty(SharedDataArgs& sharedData);
    void addConnectionPoints(SharedDataArgs& sharedData);

    bool isLeftSideJunction(SharedDataArgs& sharedData);
    bool isRightSideJunction(SharedDataArgs& sharedData);

    enum SwitchState
    {
        Left,
        Right
    };

    SwitchState m_rootSwitchState{SwitchState::Right};
    SwitchState m_middleSwitchState{SwitchState::Right};

    void doSwitch(SwitchState root, SwitchState middle);

    bool m_isLeftSide{false};
};
