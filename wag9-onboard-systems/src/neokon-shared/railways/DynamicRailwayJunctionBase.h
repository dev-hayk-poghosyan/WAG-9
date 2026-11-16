#pragma once

#include "DynamicRailwayPartGenerated.h"
#include "DynamicRailwayPartWithPaths.h"
#include "DynamicRailwayPartWithSleepers.h"

#include "neokon-shared/GenerateMeshFromSplineList.h"
#include "neokon-shared/metaunigine/MetaUnigineObjects.h"
#include "neokon-shared/railways/SplineNeighborConfigs.h"
#include "neokon-shared/TransformSpline.h"

#include <random>

class DynamicRailwayJunctionBase;
using DynamicRailwayJunctionBasePtr = std::shared_ptr<DynamicRailwayJunctionBase>;

/**
 * This class has all the shared functions for different types of inherited junctions
 */
class DynamicRailwayJunctionBase
    : public DynamicRailwayPartGenerated
    , public DynamicRailwayPartWithPaths
    , public DynamicRailwayPartWithSleepers
{
public:
    using GetOrCreateMetaClusterFunc = std::function<MetaUnigine::ObjectMeshClusterPtr(
        const std::string& name, const std::string& nodeName, const std::string& material,
        float visibleDistance)>;

public:
    DynamicRailwayJunctionBase(uint32_t partId, const std::string& configPath,
        std::vector<BezierSpline>&& splines,
        std::vector<SplineNeighborConfigs>&& neighborConfigs,
        const Unigine::Math::Mat4& transform);

    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;

    virtual void validateClusters(const GetOrCreateClusterFunc& getOrCreateCluster,
        const std::optional<Unigine::Math::Vec3>& center = std::nullopt) const override;

    virtual DynamicRailwayPartWithSleepers::ConstSleepersPtr getSleepers(
        int pathIdx) const override;

    void setSleepers(
        std::vector<DynamicRailwayPartWithSleepers::SleepersPtr>&& sleepers);

    const Unigine::Math::Mat4& getTransform() const { return m_transform; }
    const std::vector<BezierSpline>& getSplines() const { return m_splines; }
    MetaUnigine::ObjectMeshStaticPtr getMetaObject() const { return m_object; }

protected:
    using SplineInterval = std::pair<double, double>;

    struct ClusterInfo
    {
        std::string nodePath;
        RailwayMeshGeneratorConfig::MaterialInfo materialInfo;
        double visibleDistance{ 0. };
        std::vector<Unigine::Math::Mat4> transforms;
        std::string name;
    };

    using ClusterInfoPtr = std::shared_ptr<ClusterInfo>;

    struct RailFrogInfo
    {
        int leftInputSplineIdx{-1};
        int rightInputSplineIdx{-1};

        std::optional<SplineInterval> leftInputSplineInterval;
        std::optional<SplineInterval> rightInputSplineInterval;

        std::optional<SplineInterval> leftInputSplineMountsInterval;
        std::optional<SplineInterval> rightInputSplineMountsInterval;

        TransformSpline::NeighbourPoints crossPoint;
        MetaUnigine::ObjectPtr object;
        Unigine::Math::Mat4 transform;
        double angle{ 0.0 };
        double length{ 0.0 };

        bool isFlipped{ false };
        bool isObtuse{ false };
        bool guardRails{ false };
        std::map<int, std::array<Unigine::Math::Vec3, 2>> connections;
    };

    using RailFrogInfoPtr = std::shared_ptr<RailFrogInfo>;

    struct OutSurfaceData
    {
        std::string surfaceName;
        RailwayMeshGeneratorConfig::SurfaceGenerationData& genData;
        RailwayMeshGeneratorConfig::MaterialInfo addMaterialInfo;
    };

    struct SetMaterialChildNodeData
    {
        std::string nodeName;
        RailwayMeshGeneratorConfig::MaterialInfo materialInfo;
        std::string parentName;
        int surfaceIdx = 0;
    };

    struct CreateChildNodeData
    {
        std::string nodeName;
        std::string nodeFileName;
        Unigine::Math::Mat4 transform;
        RailwayMeshGeneratorConfig::MaterialInfo materialInfo;
    };

    struct RailInfo
    {
        std::string surfaceName;
        bool mirrorY{false};
    };

    struct GuardRailInfo
    {
        TransformSpline::Point start;
        TransformSpline::Point end;

        MetaUnigine::NodePtr node;
    };
    using GuardRailInfoRef = std::reference_wrapper<GuardRailInfo>;

    struct HeelBlockInfo
    {
        double start{0.0};
        double end{0.0};

        HeelBlockInfo(double start, double end);
    };
    using HeelBlockInfoRef = std::reference_wrapper<HeelBlockInfo>;

    struct PointRailInfo
    {
        MetaUnigine::NodePtr node;

        TransformSpline::Point start;
        TransformSpline::Point end;

        Unigine::Math::Mat4 leftTransform;
        Unigine::Math::Mat4 rightTransform;

        bool isFront{false};
        bool isLeft{false};
        double slicePoint{0.0};

        // Point rail spline in joined position
        std::shared_ptr<BezierSpline> joinedSpline;
        std::optional<TransformSpline::Point> joinedStartPoint;
    };
    using PointRailInfoRef = std::reference_wrapper<PointRailInfo>;

    struct PointRailNearInfo
    {
        double start{0.0};
        double end{0.0};
        bool rightSide{false};

        int otherSplineIdx{-1};
        double otherStart{0.0};
        double otherEnd{0.0};
    };

    struct SharedDataArgsBase
    {
        Unigine::MeshPtr mesh;

        std::map<int, RailInfo> railInfoById;

        std::map<int, BezierSpline> splineById;
        std::map<int, TransformSpline> splineSearchById;

        std::vector<int> leftOuterSplinesIds;
        std::vector<int> rightOuterSplinesIds;
        std::map<int, int> oppositeSplineIds;
        std::map<int, int> sourcePathSplineIds;

        std::list<RailFrogInfoPtr> railFrogsInfo;
        std::map<int, std::list<GuardRailInfo>> leftGuardRailsBySplineId;
        std::map<int, std::list<GuardRailInfo>> rightGuardRailsBySplineId;
        std::map<int, std::array<std::optional<PointRailInfo>, 2>> pointRailsBySplineId;
        std::map<int, std::vector<PointRailNearInfo>> pointRailsNearBySplineId;
        std::map<int, std::list<HeelBlockInfo>> heelBlocksBySplineId;

        // Stores transform of first and last generated middle sleeper
        std::optional<TransformSpline::Point> firstSleeperTransform;
        std::optional<TransformSpline::Point> lastSleeperTransform;

        std::vector<OutSurfaceData> outSurfaces;

        std::map<int, bool> switchBoxSides;

        std::unique_ptr<std::mt19937> random;
    };

    struct SplineNeighbourPoints
    {
        int splineIdx;
        TransformSpline::NeighbourPoints points;
    };

    struct SelectedKnotFile
    {
        std::string filePath;
        double railFrogAngleDegrees;
    };

    struct SelectedKnotSkinnedFile
    {
        std::string filePath;
        double railFrogAngleDegreesMin;
        double railFrogAngleDegreesMax;
    };

    /**
     * Get spline with @p idx from 'm_splines' or 'sharedData.splines', 
     * @throws std::out_of_range if spline with the given @p idx was not found.
     * 'idx' values are usually defined as enum class 'InternalSplineType' in derived classes 
     */
    const BezierSpline& getSpline(const SharedDataArgsBase& sharedData, int idx) const;

    /**
     * Set spline with @p idx to 'm_splines' or 'sharedData.splines',
     * 'idx' values are usually defined as enum class 'InternalSplineType' in derived classes 
     */
    void setSpline(SharedDataArgsBase& sharedData, int idx, BezierSpline&& newSpline);

    /**
     * Get spline search with @p idx from 'sharedData.splineSearch',
     * @throws std::out_of_range if search with the given @p idx was not found.
     */
    const TransformSpline& getSplineSearch(const SharedDataArgsBase& sharedData, int idx) const;
    /**
     * Get id of spline, laying opposite to 'splineId' from 'oppositeSplineIds',
     * @return std::nullopt if id was not found.
     */
    std::optional<int> getOppositeSplineId(const SharedDataArgsBase& sharedData,
        int splineId) const;
    /**
     * Get source path idx of spline, rail splines are made from path spline(in the middle of path) 
     */
    std::optional<int> getSrcPathIdx(const SharedDataArgsBase& sharedData, int splineId) const;

    /**
     * Add random flip by 180 degrees and +-'random' angle in Z plane to 'srcTransform'.
     */
    Unigine::Math::Mat4 addRandomTransform(SharedDataArgsBase& sharedData, double randomAngle,
        const Unigine::Math::Mat4& srcTransform);

    const ClusterInfoPtr& getOrCreateClusterInfo(const std::string& sourceModelPath,
        const RailwayMeshGeneratorConfig::MaterialInfo& materialInfo, const std::string& name,
        double visibleDistance);

    void generateCaps(const Unigine::String& name,
        const RailwayMeshGeneratorConfig::SurfaceGenerationData& config,
        const SplineNeighborConfigs& neighborConfigs, BezierSpline& spline);
    void generateEmbankment(SharedDataArgsBase& sharedData);

    void addRail(SharedDataArgsBase& sharedData, int srcPathIdx, int dstRailIdx, const char* name,
        double pathOffset, bool mirror);
    void generateRail(SharedDataArgsBase& sharedData, int splineIdx, const char* surfaceName,
        bool profileMirrorY = false, double trimStart = 0.0, std::optional<double> trimEnd = std::nullopt);
    void generateRails(SharedDataArgsBase& sharedData);

    // Frogs
    RailFrogInfoPtr findFirstRailFrog(SharedDataArgsBase& sharedData, int splineIdx,
        double fromDistance, double toDistance) const;
    RailFrogInfoPtr findRailFrogBySplines(SharedDataArgsBase& sharedData, int splineIdx1,
        int splineIdx2) const;
    RailFrogInfoPtr getRailFrogAt(SharedDataArgsBase& sharedData, int splineIdx, double distance,
        bool forMounts = false) const;
    RailFrogInfoPtr generateRailFrog(SharedDataArgsBase& sharedData, int leftInputSplineIdx,
        int rightInputSplineIdx, bool flip = false, bool isObtuse = false,
        bool validateLeftRight = true);

    /**
     * Choose skinned RailFrogNode for prepared 'railFrogData'
     * 'suffix' == "_A" for usual railFrog (used for primary intersections on all junctions)
     * 'suffix' == "_T" for obtuse railFrog (used for secondary intersections ~ on XCross)
     */
    bool chooseRailFrogNodeSkinned(SharedDataArgsBase& sharedData, RailFrogInfo& railFrogData,
        const char* suffix = "_A");
    std::optional<SelectedKnotSkinnedFile> selectKnotFileSkinned(double railFrogAngleDegrees,
        const char* suffix = "_A");

    /**
     * Choose fixed RailFrogNode for prepared 'railFrogData'
     * 'suffix' == "_A" for usual railFrog (used for primary intersections on all junctions)
     * 'suffix' == "_T" for obtuse railFrog (used for secondary intersections ~ on XCross)
     */
    bool chooseRailFrogNodeStatic(SharedDataArgsBase& sharedData, RailFrogInfo& railFrogData,
        const char* suffix = "_A");
    /**
     * Select knot file based on angle and type
     * For suffix values @see documentation to 'chooseRailFrogNode' 
     */
    std::optional<SelectedKnotFile> selectKnotFileStatic(double railFrogAngleDegrees,
        const char* suffix = "_A");
    std::optional<double> getKnotModelLength(const std::string& knotModelPath);

    std::optional<GuardRailInfoRef> getGuardAt(SharedDataArgsBase& sharedData, int splineIdx,
        double distance, std::optional<bool> onlyLeft = std::nullopt) const;
    std::optional<GuardRailInfoRef> getGuardAtInterval(SharedDataArgsBase& sharedData, int splineIdx,
        double fromDistance, double toDistance, std::optional<bool> onlyLeft = std::nullopt) const;
    void generateGuardRailsForRailFrog(SharedDataArgsBase& sharedData, RailFrogInfo& railFrogInfo,
        double closureOffset);

    std::optional<TransformSpline::NeighbourPoints> getStaticRailFrogClosurePoint(
        SharedDataArgsBase& sharedData, const RailFrogInfo& railFrogInfo, const char* childName,
        std::optional<double> offsetFromCenter = std::nullopt) const;
    std::optional<TransformSpline::NeighbourPoints> getSkinnedRailFrogClosurePoint(
        SharedDataArgsBase& sharedData, const RailFrogInfo& railFrogInfo, const char* boneName,
        std::optional<double> offsetFromCenter = std::nullopt) const;

    // PointRails

    std::optional<HeelBlockInfoRef> getHeelBlockAt(SharedDataArgsBase& sharedData, int splineIdx,
        double distance) const;

    /**
     * Build file path from config's pattern, length and side. Sample:
     * - pattern: "RU_ostryak_*_L*.node",
     * - length: 16500 in millimeters
     * - side: left
     * = "RU_ostryak_16500_LL.node"
     */
    void generateHeelBlock(SharedDataArgsBase& sharedData, int splineIdx, int otherSplineIdx,
        const TransformSpline::Point& end, bool leftSide);
    void generateSwitchRods(SharedDataArgsBase& sharedData,
        const DynamicRailwayJunctionBase::PointRailInfo& leftPointRail,
        const DynamicRailwayJunctionBase::PointRailInfo& rightPointRail,
        const char* suffix, bool initialRightTransform,
        RailwayMeshGeneratorConfig::SwitchRodsType switchRodConfigType);
    void adjustSkinnedPointRailBones(SharedDataArgsBase& sharedData, PointRailInfo& arrow,
        int splineIdx, int joinSplineIdx, TransformSpline::Point& start,
        const TransformSpline::Point& end, bool isLeft,
        const MetaUnigine::ObjectMeshSkinnedPtr& skinned, const TransformSpline::Point& joinedPoint,
        const TransformSpline::Point& endPoint, const Unigine::Math::Mat4& joinedScaledTransform);
    std::optional<std::reference_wrapper<PointRailInfo>>
    generatePointRail(SharedDataArgsBase& sharedData, int splineIdx, int joinSplineIdx,
        TransformSpline::Point& start, const TransformSpline::Point& end,
        bool isLeftType, bool isFront, const char* nodeName, bool scale = true);

    enum class SleepersIntervalType
    {
        Default = 1,
        GuardRails = 2,
        PointRailsFront = 3,
        PointRailsBack = 4
    };

    using ModifySleeperWithLengthFuncType = std::function<void(double, Unigine::Math::Mat4&)>;
    using AddSleeperWithLengthFuncType = std::function<void(double, const Unigine::Math::Mat4&)>;
    using AddSleeperInfoFuncType = std::function<void(const Unigine::Math::Mat4& transform)>;
    using AddMountsFuncType = std::function<void(const Unigine::Math::Mat4& middle, double distance,
        const SplineNeighbourPoints& left, const SplineNeighbourPoints& right)>;
    using AddMountsFuncs = std::vector<AddMountsFuncType>;

    struct MountTransform
    {
        Unigine::Math::Mat4 transform;
        bool underPointRail{false};
    };

    std::optional<Unigine::Math::Mat4> calcSingleSideMountTransform(
        const SharedDataArgsBase& sharedData, int splineIdx,
        const TransformSpline::Point& point) const;
    std::optional<MountTransform> calcMountTransform(SharedDataArgsBase& sharedData, int splineIdx,
        const TransformSpline::Point& point) const;

    using DoAddMiddleSleepersIntervalFuncType = std::function<void(double startDst, double endDst, double step, int type,
        bool skipFirst, bool skipLast, const AddMountsFuncs& addMountsFuncs)>;
    void doAddMiddleSleepersDefault(SharedDataArgsBase& sharedData, const TransformSpline& middleSpline, double startDst, double endDst,
        double step, int type, bool skipFirst, bool skipLast, const std::vector<int>& leftOuterSplinesIds,
        const std::vector<int>& rightOuterSplinesIds, const AddMountsFuncs& addMountsFuncs,
        const AddSleeperInfoFuncType& addSleeperInfoFunc,
        const ModifySleeperWithLengthFuncType& modifySleeperFunc = {});
    void doAddMiddleSleepersSwitchBoxOverride(SharedDataArgsBase& sharedData,
        const TransformSpline& middleSpline, double startDst, double endDst, double step, int type,
        bool skipFirst, bool skipLast, const std::vector<int>& leftOuterSplinesIds,
        const std::vector<int>& rightOuterSplinesIds, int switchRodsType, bool invertBoxSide,
        const AddMountsFuncs& addMountsFuncs,
        const AddSleeperInfoFuncType& addSleeperInfoFunc,
        const ModifySleeperWithLengthFuncType& modifySleeperFunc = {});

    void generateMiddleSleepers(SharedDataArgsBase& sharedData, const TransformSpline& middleSpline,
        const std::vector<int>& railSplineIds, const DoAddMiddleSleepersIntervalFuncType& doAddSleepersFunc);
    void generateMiddleSleepers(SharedDataArgsBase& sharedData, const TransformSpline& middleSpline,
        const std::vector<int>& railSplineIds, const AddSleeperInfoFuncType& addSleeperInfoFunc = {});

    using GenerateSleeperFuncType = std::function<bool(double dst)>;
    bool generateSleepers(double& currentDistance, double lastDistance, double baseStep, bool skipFirst, bool skipLast,
        const GenerateSleeperFuncType& genSleeperFunc);

    bool generateMiddleSplineSleeper(SharedDataArgsBase& sharedData,
        const TransformSpline& middleSpline, double distance,
        const std::vector<int> leftOuterSplinesIds,
        const std::vector<int> rightOuterSplinesIds,
        const AddSleeperWithLengthFuncType& addSleepersFunc,
        const AddMountsFuncs& addMountsFuncs = {});
    bool generateMiddleSplineSleeperDefault(SharedDataArgsBase& sharedDat,
        const TransformSpline& middleSpline, double distance,
        const std::vector<int> leftOuterSplinesIds,
        const std::vector<int> rightOuterSplinesIds,
        const AddSleeperInfoFuncType& addSleeperInfoFunc,
        const AddMountsFuncs& addMountsFuncs = {},
        const ModifySleeperWithLengthFuncType& modifySleeperFunc = {});
    /**
     * Generate sleepers on single path 'splineIdx' starting from the last generated middle sleeper
     * position 'startMiddleTransform' to 'endDistance' (can be negative, evaluates to
     * spline.arcLength() - endDistance).
     */
    void generateSleepersOnOut(SharedDataArgsBase& sharedData, int splineIdx,
        const Unigine::Math::Mat4& startMiddleTransform, double endDistance,
        const AddSleeperInfoFuncType& addSleeperFunc = {});

    void generateRailSurfaces(SharedDataArgsBase& sharedData, GenerateMeshFromSplineArgs& args);
    bool generateSurface(SharedDataArgsBase& sharedData,
        RailwayMeshGeneratorConfig::SurfaceGenerationData& config,
        GenerateMeshFromSplineArgs& args);

    void setupMeshMaterials(SharedDataArgsBase& sharedData);
    void setupMaterialsForRailFrog(SharedDataArgsBase& sharedData, const RailFrogInfo& railFrog,
        MetaUnigine::ObjectMeshStaticPtr const& junctionMesh);

    /**
     * In case if surface gen config has replacement textures for material, create and assign new 
     * temporary materials with them. Can be called only from main thread.
     */
    void applyMaterialTextures(const Unigine::NodePtr& node);

    /**
     * This functions search for nearest point of the 'splineIds' splines and select the one
     * with minimal offset from 'from' point.
     */
    std::optional<SplineNeighbourPoints> findFirstNearestSplinePoint(
        SharedDataArgsBase& sharedData, const std::vector<int>& splineIds, const glm::dvec3& from,
        const glm::dvec3& to, double maxDistance = 0.2);

    /**
     * This functions search for nearest point of the 'splineIds' splines and select the one
     * with minimal distance to 'from -> to' vector.
     */
    std::optional<TransformSpline::NeighbourPoints> findNearestSplinePoint(
        SharedDataArgsBase& sharedData, const std::vector<int>& splineIds, const glm::dvec3& from,
        const glm::dvec3& to);

    void setMetaObject(const MetaUnigine::ObjectMeshStaticPtr& object) { m_object = object; }
    void swapSplines(int firstIdx, int secondIdx);

    /**
     * This functions returns vector of int, filled with interpolated values from 'first' to 'last', including ends.
     */
    std::vector<int> interFillVec(int first, int last);
    /**
     * This functions returns united vector of int, with all values from 'first' and 'last' vector.
     */
    std::vector<int> uniteVecs(const std::vector<int>& first, const std::vector<int>& last);

private:
    std::vector<ClusterInfoPtr> m_clusters;
    MetaUnigine::ObjectMeshStaticPtr m_object;
    std::vector<BezierSpline> m_splines;
    std::vector<SplineNeighborConfigs> m_neighborConfigs;
    Unigine::Math::Mat4 m_transform;

    std::vector<OutSurfaceData> m_temporaryMaterialsInfo;
    std::vector<CreateChildNodeData> m_createChildNodesInfo;
    std::vector<SetMaterialChildNodeData> m_setChildNodeMaterialInfo;

    mutable std::mutex m_sleepersInfosProtect;
    std::vector<DynamicRailwayPartWithSleepers::SleepersPtr> m_sleepersInfos;
};
