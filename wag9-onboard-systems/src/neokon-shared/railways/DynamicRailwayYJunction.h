#pragma once

#include "neokon-shared/GenerateMeshFromSplineList.h"

#include "neokon-shared/TransformSpline.h"
#include "neokon-shared/railways/DynamicRailwayJunctionBase.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"

#include <UnigineMathLib.h>
#include <UnigineMesh.h>
#include <UnigineObjects.h>

class DynamicRailwayYJunction;
using DynamicRailwayYJunctionPtr = std::shared_ptr<DynamicRailwayYJunction>;

/**
 * This class is used by 'DynamicRailways' to load/unload and create nearby railway Y junctions
 * in runtime. @see documentation to base class DynamicRailwayPart.
 */
class DynamicRailwayYJunction : public DynamicRailwayJunctionBase
{
public:
    DynamicRailwayYJunction(uint32_t partId, const std::string& configPath,
        std::vector<BezierSpline>&& splines,
        std::vector<SplineNeighborConfigs>&& neighborConfigs,
        const Unigine::Math::Mat4& transform);

    virtual bool load() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual Unigine::NodePtr createNodes() override;

    void switchToRight();
    void switchToLeft();
    bool isSwitchedToRight() const;

private:
    enum class InternalSplineType
    {
        PathLeft = 0,
        PathRight,
        PathLeft_LeftRail,
        PathLeft_RightRail,
        PathRight_LeftRail,
        PathRight_RightRail,
        Total
    };

    struct SharedDataArgs : SharedDataArgsBase
    {
        TransformSpline middleSpline;
        TransformSpline::Point middleCrossPoint;
        TransformSpline::NeighbourPoints closureCrossPoint;

        double railProfileWidth{ 0.0 };
        double guardRailClosureOffset{ 0.0 };

        std::vector<DynamicRailwayPartWithSleepers::SleepersPtr> sleepersInfos;
    };

    bool buildSplines(SharedDataArgs& sharedData);

    void generateSleepersOnOuts(SharedDataArgs& sharedData);
    bool generatePointRails(SharedDataArgs& sharedData);

    void addJunctionProperty(SharedDataArgs& sharedData);
    void addConnectionPoints(SharedDataArgs& sharedData);

    void generateLODs(SharedDataArgs& sharedData, GenerateMeshFromSplineArgs& args);

    enum SwitchState
    {
        Left,
        Right
    };

    SwitchState m_switchState{SwitchState::Right};
};
