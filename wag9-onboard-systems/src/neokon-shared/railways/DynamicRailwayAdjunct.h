#pragma once

#include "DynamicRailwayPathBaseAdjunct.h"

class DynamicRailwayAdjunct;
using DynamicRailwayAdjunctPtr = std::shared_ptr<DynamicRailwayAdjunct>;

/**
 * Класс для динамической генерации модели контактного рельса.
 * Контактный рельс может быть сгенерирован справа или слева от пути по ходу
 * движения поезда.
 */
class DynamicRailwayAdjunct : public DynamicRailwayPathBaseAdjunct
{
public:
    static const char* ContactRailsSurfaceNameLOD0;
    static const char* ContactRailsSurfaceNameLOD1;

public:
    /**
     * @param partId - уникальный ИД объекта, для генерации имени сгенерированной ноды.
     * @param configPath - полный путь до конфигурационного файла с параметрами генерации.
     * @param spline - рабочий сплайн, опорные точки которого заданы в мировых координатах.
     * @param contactRailType - тип генериуемого контактного (правосторонний или левосторонний).
     */

    using CheckAlignedBoxWorldVisibilityFunc =
        std::function<bool(const AlignedBox<glm::dvec3>& boxInWorld)>;

    static DynamicRailwayAdjunctPtr create(
        uint32_t partId, uint32_t segmentIdx, const std::string& configPath, double startDistanceM,
        double endDistanceM, const BezierSpline& bezierInWorld, double adjunctFullLength,
        double adjunctStartOffsetMM, bool lastSegment,
        const DynamicRailwayPartWithSleepers::ConstSleepersPtr& sleepers);

    explicit DynamicRailwayAdjunct(uint32_t partId, uint32_t segmentIndex,
        const std::string& meshGenConfigPath, double startDistanceM, double endDistanceM,
        const BezierSpline& bezierInWorld, double adjunctFullLengthM, double adjunctStartOffsetM,
        bool lastSegment, const DynamicRailwayPartWithSleepers::ConstSleepersPtr& sleepers);

    virtual bool load() override;
    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual void validateClusters(GetOrCreateClusterFunc const& getOrCreateCluster,
                 std::optional<Unigine::Math::Vec3> const& center = std::nullopt) const override;

private:
    bool loadPart(const RailwayMeshGeneratorConfig::AdjunctPartData& part,
                  std::optional<Unigine::Math::Mat4>& nodeTransform);

    bool isLeftSide(const RailwayMeshGeneratorConfig::AdjunctPartData& part) const;
    Unigine::Math::Vec3 getOffsetVector(
        const RailwayMeshGeneratorConfig::AdjunctPartData& part) const;

    double m_adjunctStartOffsetM{0.0};
    double m_adjunctFullLengthM{0.0};

    std::map<std::string, std::vector<Unigine::Math::Mat4>> m_nodesWithTransforms;
};

