#pragma once

#include "neokon-shared/BezierSegment.h"
#include "neokon-shared/railways/DynamicRailwayPartGenerated.h"
#include "neokon-shared/railways/DynamicRailwayPartWithPaths.h"
#include "neokon-shared/railways/DynamicRailwayPartWithSleepers.h"
#include "neokon-shared/railways/RailwayMeshGeneratorConfig.h"
#include "neokon-shared/railways/SplineNeighborConfigs.h"

#include <UnigineMathLib.h>
#include <UnigineMesh.h>
#include <UnigineObjects.h>

class DynamicRailwayBranchSegment;
using DynamicRailwayBranchSegmentPtr = std::shared_ptr<DynamicRailwayBranchSegment>;

/**
 * This class is used by 'DynamicRailways' to load/unload and create nearby railway branch segments
 * in runtime. @see documentation to base class DynamicRailwayPart.
 */
class DynamicRailwayBranchSegment
    : public DynamicRailwayPartGenerated
    , public DynamicRailwayPartWithPaths
    , public DynamicRailwayPartWithSleepers
{
public:
    static const char* EmbankmentSurfaceName;
    static const char* RailsSurfaceNameLOD0;
    static const char* RailsSurfaceNameLOD1;
    static const double SleepersPerTexture;

    using CheckAlignedBoxWorldVisibilityFunc =
        std::function<bool(const AlignedBox<glm::dvec3>& boxInWorld)>;

public:
    /**
     * This functions splits 'bezierInWorld' to subSegments based on config's 'max_segment_length'
     * value, 'segmentIdx' is limited to 16bits because full 32bit 'subId' of DynamicRailwayPart is
     * splitted to store segmentIdx and subSegmentIdx.
     */
    static void createAndAppendSplittedSegments(
        const std::function<void(const DynamicRailwayPartPtr&)>& appendFunc, uint32_t branchId,
        uint16_t segmentIdx, const std::string& configPath,
        const SplineNeighborConfigs& neighborConfigPaths, double startDistanceM, bool isLast,
        const BezierSegment& bezierInWorld,
        const boost::uuids::uuid& pathUUID,
        const CheckAlignedBoxWorldVisibilityFunc& checkVisibilityFunc = {});

    static DynamicRailwayBranchSegmentPtr create(uint32_t branchId, uint32_t segmentIdx,
        const std::string& configPath, const SplineNeighborConfigs& neighborConfigPaths,
        double startDistanceM, bool isLast, const BezierSegment& bezierInWorld,
        const boost::uuids::uuid& pathUUID, const RailwayMeshGeneratorConfigPtr& config = nullptr);

    DynamicRailwayBranchSegment(uint32_t branchId, uint32_t segmendIdx,
        const std::string& configPath, const SplineNeighborConfigs& neighborConfigPaths,
        double startDistanceM, bool isLast, const BezierSegment& bezierInWorld,
        const RailwayMeshGeneratorConfigPtr& config);

    virtual bool load() override;
    virtual void unload() override;

    virtual Unigine::NodePtr createNodes() override;
    virtual bool isDifferentWith(const DynamicRailwayPartPtr& old) const override;
    virtual void validateClusters(const GetOrCreateClusterFunc& getOrCreateCluster,
        const std::optional<Unigine::Math::Vec3>& center = std::nullopt) const override;

    virtual DynamicRailwayPartWithSleepers::ConstSleepersPtr getSleepers(
        int pathIdx) const override;

    template<typename ObjType>
    void applyObjectParameters(const ObjType& object) const;
    Unigine::MeshPtr getMesh() const { return m_mesh; }

    static double generateSleeperTransforms(const BezierSegment& segment, double pathShift,
        double pathStep, bool trimEnd, double distance,
        const std::function<void(const Unigine::Math::Mat4&)>& addTransform);

protected:
    struct CapInfo
    {
        std::string capNodePath;
        const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo;
        std::vector<Unigine::Math::Mat4> transforms;
    };

    CapInfo& addCapInfo(const std::string& capNodePath,
        const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo);

    void generateEmbankment(const RailwayMeshGeneratorConfig::SurfaceGenerationData& config);
    Unigine::Vector<Unigine::Math::Mat4> generateRails(const char* surfaceName,
        RailwayMeshGeneratorConfig::SurfaceGenerationData& config, double gapStepM = -1,
        double gapLengthM = 0.01);
    void generateOtherSurfaces();
    void generateCaps(const RailwayMeshGeneratorConfig::SurfaceGenerationData& config,
                      const BezierSegment& segment);

    int getOrCreateMeshSurface(const char* name);

    void applyBlendEmbankment(const Unigine::ObjectMeshStaticPtr& object);

private:
    SplineNeighborConfigs m_neighborConfigPaths;

    BezierSegment m_bezierSegment;

    const double m_startDistanceM;
    double m_railOffsetM = 0.;
    double m_sleepersStepM = 0.;
    double m_uvCorrection = 1.;

    const bool m_segmentIsLast;
    Unigine::Math::Mat4 m_segmentTransform;

    Unigine::MeshPtr m_mesh;
    std::vector<Unigine::Vector<Unigine::Math::Mat4>> m_sleeperPositions;
    Unigine::Vector<Unigine::Math::Mat4> m_fishPlatePositions;
    std::vector<CapInfo> m_capInfos;

    mutable std::mutex m_sleepersInfosProtect;
    std::vector<DynamicRailwayPartWithSleepers::SleepersPtr> m_sleepersInfos;
};

template<typename ObjType>
void DynamicRailwayBranchSegment::applyObjectParameters(const ObjType& object) const
{
    object->setWorldTransform(m_segmentTransform);

    auto& commonConfig = m_config->getCommonData();
    auto& embankmentConfig = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::EmbankmentLOD0);
    auto& railConfigLOD0 = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::RailsLOD0);
    auto& railConfigLOD1 = m_config->getSurfaceData(
        +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::RailsLOD1);

    embankmentConfig.applySurfaceParams(object, EmbankmentSurfaceName,
        commonConfig.sleepersMaterialInfo);
    railConfigLOD0.applySurfaceParams(object, RailsSurfaceNameLOD0);
    railConfigLOD1.applySurfaceParams(object, RailsSurfaceNameLOD1);

    const size_t surfacesCount = m_config->getSurfacesData().size();
    for (int surfaceIndex = +RailwayMeshGeneratorConfig::GeneratedSurfaceIdx::Total;
        surfaceIndex < surfacesCount; ++surfaceIndex)
    {
        const auto& config = m_config->getSurfaceData(surfaceIndex);
        config.applySurfaceParams(object, config.name.data());
    }
}
