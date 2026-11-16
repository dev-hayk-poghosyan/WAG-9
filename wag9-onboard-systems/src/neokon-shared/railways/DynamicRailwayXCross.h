#pragma once

#include "neokon-shared/GenerateMeshFromSplineList.h"

#include "neokon-shared/TransformSpline.h"
#include "neokon-shared/railways/DynamicRailwayJunctionBase.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"

#include <UnigineMathLib.h>
#include <UnigineMesh.h>
#include <UnigineObjects.h>

class DynamicRailwayXCross;
using DynamicRailwayXCrossPtr = std::shared_ptr<DynamicRailwayXCross>;

/**
 * This class is used by 'DynamicRailways' to load/unload and create nearby railway Y junctions
 * in runtime. @see documentation to base class DynamicRailwayPart.
 */
class DynamicRailwayXCross : public DynamicRailwayJunctionBase
{
public:
    DynamicRailwayXCross(uint32_t partId, const std::string& configPath,
        std::vector<BezierSpline>&& splines,
        std::vector<SplineNeighborConfigs>&& neighborConfigs,
        const Unigine::Math::Mat4& transform);

    virtual bool load() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;

private:
    /**
     * Типы рабочих сплайнов по которым происходит построение стрелки.
     */
    enum class InternalSplineType
    {
        PathLeftToRight = 0, //< левый путь
        PathRightToLeft = 1, //< правый путь

        PathLeftToRight_LeftRail, //< левый путь - левый рельс
        PathLeftToRight_RightRail,//< левый путь - правый рельс
        PathRightToLeft_LeftRail, //< правый путь - левый рельс
        PathRightToLeft_RightRail, //< правый путь - правый рельс.
        Total
    };

    /**
     * Структура для хранения промежуточных данных,
     * используемых различными этапами генерации стрелки.
     */
    struct SharedDataArgs : SharedDataArgsBase
    {
        TransformSpline middleSpline;

        // Ширина пути (расстояние между центрами рельс)
        double trackWidth{0.0};

        //Точка пересечения осей двух путей стрелки. Опорная точка стрелки.
        TransformSpline::Point middleCrossPoint;

        //Отступ для контррельс от оси пути в метрах.
        double guardRailClosureOffset{ 0.0 };

        std::vector<DynamicRailwayPartWithSleepers::SleepersPtr> sleepersInfos;
    };

    std::optional<TransformSpline::NeighbourPoints> validateInputSplinesAndFindCrossPoint(
        SharedDataArgs& sharedData);

    bool buildSplines(SharedDataArgs& sharedData);
    void generateSleepersOnOuts(SharedDataArgs& sharedData);
    void generateSingleSleepers(SharedDataArgs& sharedData, double angleDeg);

    void addJunctionProperty(SharedDataArgs& sharedData);
    void addConnectionPoints(SharedDataArgs& sharedData);
};
