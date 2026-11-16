#pragma once

#include "neokon-shared/GenerateMeshFromSplineList.h"

#include "neokon-shared/TransformSpline.h"
#include "neokon-shared/railways/DynamicRailwayJunctionBase.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"

#include <UnigineMathLib.h>
#include <UnigineMesh.h>
#include <UnigineObjects.h>

class DynamicRailwayXJunction;
using DynamicRailwayXJunctionPtr = std::shared_ptr<DynamicRailwayXJunction>;

/**
 * This class is used by 'DynamicRailways' to load/unload and create nearby railway X junctions
 * in runtime.
 * 'splitSwitch' parameter enables non-simultaneous movement of both front/both back switches,
 * allowing to switch left-to-left and right-to-right simultaneously via 'switchStraightDOnly' call,
 * this parameter can be overritten by load() function in case junction has incompatible splines.
 * 'switchStraightDOnly' will only work with 'splitSwitch' == true.
 * 'switchLeftToLeftXOnly' and 'switchRightToRightXOnly' are always working, but seemless for wide
 * junctions with both side ways.
 * @see documentation to base class DynamicRailwayPart.
 */
class DynamicRailwayXJunction : public DynamicRailwayJunctionBase
{
public:
    /**
     * Типы рабочих сплайнов по которым происходит построение стрелки.
     */
    enum class InternalSplineType
    {
        PathLeftToLeft = 0,
        PathLeftToRight,
        PathRightToLeft,
        PathRightToRight,

        RailStockLeft,
        RailClosureFrontLeft,
        RailClosureBackLeft,

        RailMiddleRightToRight,
        RailMiddleLeftToLeft,

        RailClosureFrontRight,
        RailClosureBackRight,
        RailStockRight,

        Total
    };

    DynamicRailwayXJunction(uint32_t partId, const std::string& configPath,
        std::vector<BezierSpline>&& splines,
        std::vector<SplineNeighborConfigs>&& neighborConfigs,
        const Unigine::Math::Mat4& transform, bool splitSwitch);

    virtual bool load() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual Unigine::NodePtr createNodes() override;

    bool isSplitSwitch() const { return m_splitSwitch; };

    void switchStraightDOnly();

    void switchLeftToRight();
    void switchRightToLeft();

    void switchLeftToLeftXOnly();
    void switchRightToRightXOnly();

private:
    enum SwitchState
    {
        Left,
        Right
    };

    enum class NarrowPointRailIndex
    {
        LeftStock = 0,
        LeftMiddle,
        RightMiddle,
        RightStock,
        Total
    };

    using NarrowPointRails = std::array<std::optional<PointRailInfo>, +NarrowPointRailIndex::Total>;

    /**
     * Структура для хранения промежуточных данных,
     * используемых различными этапами генерации стрелки.
     */
    struct SharedDataArgs : SharedDataArgsBase
    {
        TransformSpline middleSpline;

        //Точка пересечения осей двух путей стрелки. Опорная точка стрелки.
        TransformSpline::Point middleCrossPoint;

        double railOffset{0.0};

        //Ширина профиля рельса в метрах.
        double railProfileWidth{0.0};

        //Отступ для контррельс от оси пути в метрах.
        double guardRailClosureOffset{ 0.0 };

        std::vector<DynamicRailwayPartWithSleepers::SleepersPtr> sleepersInfos;
    };

    bool buildSplines(SharedDataArgs& sharedData);

    void generateNarrowPointRailsForSharpFrog(SharedDataArgs& sharedData,
        const RailFrogInfo& sharpFrog, const RailFrogInfo& leftFrog, const RailFrogInfo& rightFrog);
    void generateWidePointRails(SharedDataArgs& sharedData, int pointSplineIdx, int joinSplineIdx,
        bool pointSplineIsLeft, RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType);
    void generateNarrowSwitchBox(const Unigine::Math::Mat4& closureTransform, bool isFlipped,
                                 std::optional<bool> forceLeft);
    void generateNarrowBorderBar(const std::optional<PointRailInfo>& leftPointRail,
        const std::optional<PointRailInfo>& rightPointRail, bool isFlipped);
    void generateNarrowSwitchRods(SharedDataArgs& sharedData, const NarrowPointRails& pointRails,
        bool isFlipped);
    void snapNarrowSwitchRodParts(const NarrowPointRails& pointRails,
        const MetaUnigine::NodePtr& rodNode,
        const char* nameSuffixToAdd);

    std::optional<TransformSpline::NeighbourPoints> findMiddlePointRailsStart(
        SharedDataArgs& sharedData,
        const RailFrogInfo& frog);
    std::optional<TransformSpline::Point> findOppositeRailPoint(SharedDataArgs& sharedData,
        int stockSplineIdx, bool isLeft, const Unigine::Math::Mat4& closureTransform);

    const PointRailNearInfo* getNearPointRail(SharedDataArgs& sharedData, bool isFront, int splineIdx);

    std::pair<std::optional<TransformSpline::Point>, std::optional<TransformSpline::Point>>
    generateWideSleepers(SharedDataArgs& sharedData, bool isFront, int primarySplineIdx,
        const std::vector<int>& leftOuterSplinesIds, const std::vector<int>& rightOuterSplinesIds,
        int switchRodsType);

    void generateSleepersOnXOut(SharedDataArgs& sharedData, int splineIdx, int otherSplineIdx,
        const Unigine::Math::Mat4& lastWideSleeperTransform, double endDistance);
    void generateSleepersOnOuts(SharedDataArgs& sharedData, bool isLeftNarrow, bool isRightNarrow);

    void addJunctionProperty(SharedDataArgs& sharedData);
    void addConnectionPoints(SharedDataArgs& sharedData);

    bool isLeftNarrowJunction(SharedDataArgs& sharedData,
        const Unigine::Math::Mat4& leftFrogTransform,
        const Unigine::Math::Mat4& rightFrogTransform);

    bool isRightNarrowJunction(SharedDataArgs& sharedData,
        const Unigine::Math::Mat4& leftFrogTransform,
        const Unigine::Math::Mat4& rightFrogTransform);

    void doSwitch2(SwitchState front, SwitchState back);
    void doSwitch4(SwitchState frontLeft, SwitchState frontRight, SwitchState backLeft,
        SwitchState backRight);

    SwitchState m_frontLeftSwitchState{SwitchState::Right};
    SwitchState m_frontRightSwitchState{SwitchState::Right};
    SwitchState m_backLeftSwitchState{SwitchState::Right};
    SwitchState m_backRightSwitchState{SwitchState::Right};
    bool m_splitSwitch = false;
};
