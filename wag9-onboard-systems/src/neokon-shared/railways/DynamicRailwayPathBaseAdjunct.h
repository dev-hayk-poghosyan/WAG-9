#pragma once

#include "DynamicRailwayPartGenerated.h"
#include "DynamicRailwayPartWithSleepers.h"

#include "neokon-shared/TransformSpline.h"
#include "neokon-shared/metaunigine/MetaUnigineObjects.h"
#include "neokon-shared/GenerateMeshFromSplineList.h"

/**
 * Базовый класс для генерации "адьюнктов" - объектов-модификаторов
 * или дополнений к железнодородным путям. Класс является полуфабрикатом,
 * частично реализающим операции, специфичные для адьюнктов, и должнен быть расширен
 * конкретными подклассами.
 */
class DynamicRailwayPathBaseAdjunct : public DynamicRailwayPartGenerated
{
public:
    DynamicRailwayPathBaseAdjunct(uint32_t partId, std::string const& configPath,
        BezierSpline const& spline);

    DynamicRailwayPathBaseAdjunct(uint32_t partId, uint32_t segmentIndex,
        const std::string& meshGenConfigPath, double startDistanceM, double endDistanceM,
        const BezierSpline& bezierInWorld, bool firstSegment, bool lastSegment,
        const DynamicRailwayPartWithSleepers::ConstSleepersPtr& sleepers);

    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual void unload() override;

protected:
    void addTransformsToCluster(ObjectMultiMeshClusterPtr const& cluster,
                                std::optional<Unigine::Math::Vec3>  const& genAreaCenter,
                                 std::vector<Unigine::Math::Mat4> const& transforms) const;
    int getOrCreateMeshSurface(const char* name);
    Unigine::ObjectMeshStaticPtr doCreateNode(const char* name) const;

    BezierSpline m_srcSpline; // Full railway part path spline, not trimmed here to speed up creation
    const double m_startDistanceM; // Trim start distance to be applied to m_srcSpline at load time
    const double m_endDistanceM;   // Trim end distance to be applied to m_srcSpline at load time

    Unigine::Math::Mat4 m_nodeTransform;
    Unigine::MeshPtr m_mesh;

    double m_railOffsetM = 0.;
    double m_sleeperStepM = 0.;
    double m_uvCorrection = 1.;

    const bool m_firstSegment;
    const bool m_lastSegment;

    DynamicRailwayPartWithSleepers::ConstSleepersPtr m_sleepers;
};

