#pragma once

#include "MeshGenConfig.pb.h"
#include "neokon-shared/PlusEnum.h"
#include "neokon-shared/IntersectionMasks.h"

#include <UnigineXml.h>
#include <UnigineObjects.h>

#include <array>
#include <mutex>
#include <optional>

class RailwayMeshGeneratorConfig;
using RailwayMeshGeneratorConfigWeak = std::weak_ptr<RailwayMeshGeneratorConfig>;
using RailwayMeshGeneratorConfigPtr = std::shared_ptr<RailwayMeshGeneratorConfig>;

/**
 * This class is used by RailwayBranchMeshGenerator, RailwayYJunctionMeshGenerator and
 * DynamicRailwayPartGenerated's inherited classes as parameters for generation of railway parts.
 * @see http://git.neokon.local:22580/root/GlobalTerrainEditor-release/wikis/Control-parameters-of-mesh-generation-for-branches-and-junctions 
 * wiki page for details. All the int32 parameters are in millimeter units and should be converted to
 * meters by toInnerUnits() call.
 */
class RailwayMeshGeneratorConfig
{
public:
    static const double toInnerUnitsScale; //Millimeters -> Metters
    static const double toConfigUnitsScale; //Meters -> Millimeters

    /**
     * PreDefined generated mesh surface ids 
     */
    enum class GeneratedSurfaceIdx
    {
        EmbankmentLOD0 = 0,
        EmbankmentJunctions,
        RailsLOD0,
        RailsLOD1,
        ContactRailLOD0,
        ContactRailLOD1,
        GuardRailLOD0,
        GuardRailLOD1,
        Total
    };

    /**
     * SwitchRod ids
     */
    enum class SwitchRodIndex
    {
        RodHead,
        RodMinusOne,
        Rod0,
        Rod1,
        Rod2,
        Total
    };

    /**
     * SwitchRods type
     */
    enum class SwitchRodsType
    {
        Default = 0,
        XFront,
        XBack,
        XFrontLeft,
        XFrontRight,
        XBackLeft,
        XBackRight,
        TWFirst,
        TWSecond,
        Total
    };

    /**
     * Pair of name-value for material info
     */
    struct MaterialParam
    {
        std::string name;
        std::string value;
    };

    struct MaterialInfo
    {
        void loadFromProto(const proto::meshgen::MaterialInfo& source);
        void saveToProto(proto::meshgen::MaterialInfo& source) const;

        // This function can be used only in main thread to create temporary material
        Unigine::MaterialPtr getOrCreateMaterial(const MaterialInfo& addParams = {}) const;

        bool hasParamsToApply() const;
        Unigine::String generateMatNameFromParams() const;

        void apply(const Unigine::MaterialPtr& mat) const;
        void applyTextures(const Unigine::MaterialPtr& mat,
            const std::string& texPrefix = {}) const;

        bool operator==(const MaterialInfo& other) const;

        std::string name;

        std::vector<MaterialParam> states;
        std::vector<MaterialParam> textures;
        std::vector<MaterialParam> params;
    };

    /**
     * Paramers, used by all generators
     */
    struct CommonGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::Common& source);
        void saveToProto(proto::meshgen::Common& source) const;

        int railwayTrackWidth{ 1520 };
        int maxPathStraightTreshold{ 100 };
        int minPathSimilarityPercent{ 95 };

        MaterialInfo sleepersMaterialInfo;
        MaterialInfo blendEmbankmentMaterialInfo;
    };

    /**
     * Paramers, used only by branch generators
     */
    struct BranchCommonGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::BranchCommon& source);
        void saveToProto(proto::meshgen::BranchCommon& source) const;

        // Add gap on every 'gapStep' meters, with 'gapLength' width, '-1' = no gaps
        double gapStep{ -1.0 };
        double gapLength{ 0.01 };

        // fishPlate node to be placed on every gap
        std::string fishPlateNode;
        MaterialInfo fishPlateMaterialInfo;

        double maxSegmentLength{ 100.0 };
    };

    /**
     * Point Profile Data
     */
    struct SurfaceGenerationData; // Forward declaration

    struct PointProfileData
    {
        friend SurfaceGenerationData;

    public:
        void loadFromProto(const proto::meshgen::PointProfile& source);
        void saveToProto(proto::meshgen::PointProfile& source) const;

        void loadPoints(
            const google::protobuf::RepeatedPtrField<proto::meshgen::ProfilePoint>& source);

        bool arePointsDifferent(const PointProfileData& other) const;

        std::string name; // optional surface name

        std::vector<Unigine::Math::Vec3> points; // Profile points
        std::vector<Unigine::Math::vec3> normals; // Profile normals
        std::vector<Unigine::Math::vec2> uvCoords; // Profile UVs

    private:
        void validateNormals();
    };

    using PointProfileDataPtr = std::shared_ptr<PointProfileData>;

    /**
     * Paramers for surface generation
     */
    struct SurfaceGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::Surface& source,
            std::vector<PointProfileDataPtr>& pointProfiles);
        void saveToProto(proto::meshgen::Surface& source) const;

        // Main thread only
        template<typename ObjType>
        bool applySurfaceParams(const ObjType& object, const char* surfaceName,
            const MaterialInfo& addMaterialParams = {}) const;

        const std::vector<std::array<int, 3>>& getCapTriangles();

        std::string name; // optional surface name

        Unigine::Math::vec3 profileOffset;
        PointProfileDataPtr pointProfile;

        double numSegmentsPerUnit{ 3.0 }; // Count segments per meter
        double texCoordVStep{ 0.2 }; // 'v' tex coord step per meter

        double minVisibleDistance{0.0};
        double maxVisibleDistance{ 100.0 };
        double minFadeDistance{ 10.0 };
        double maxFadeDistance{ 100.0 };

        // material to be assigned to generated surface if there is no material for that surface in
        // 'customMaterials'
        MaterialInfo defaultMaterialInfo;

        // material to be assigned to generated surfaces
        std::unordered_map<std::string, std::string> customMaterials;

        std::string capNodePath;
        int collisionMask{0};

    private:
        void loadCustomMaterials(const proto::meshgen::Surface& source);

        std::mutex m_capTrianglesProtect;
        std::vector<std::array<int, 3>> m_capTriangles; // Profile cap triangles
    };

    using SurfaceGenerationDataUnique = std::unique_ptr<SurfaceGenerationData>;
    using SurfaceGenerationDataPtr = std::shared_ptr<SurfaceGenerationData>;

    /**
     * Paramers for junction sleepers generation
     */
    struct JunctionSleepersGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::JunctionSleepers& source);
        void saveToProto(proto::meshgen::JunctionSleepers& source) const;

        std::string directory;
        std::string nodeNamePattern;
        std::string mountNodePath;
        std::string singleSidedMountNodePath;

        double visibleDistance{ 100.0 };
        int minLength{ 3000 };
        int maxLength{ 7000 };
        int lengthIncreaseStep{ 250 };
        int startOffset{ 250 };
        int interval{ 500 };
        int switchBoxInterval{600};
        int pointRailsFirstInterval{ 600 };
        int sleeperWidth{230};
        bool addRandomAngle{true}; // Should the random angle be added
        double randomAngle{0.0}; // Random angle to add to sleeper rotation in Z axis
        std::string firstJunctionSleeper;
        std::string secondJunctionSleeper;

        MaterialInfo defaultMaterialInfo;
    };

    /**
     * Paramers for branch sleepers generation
     */
    struct BranchSleepersGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::BranchSleepers& source);
        void saveToProto(proto::meshgen::BranchSleepers& source) const;

        std::string getTemplatePath(int variant) const; // Build template node path for variant index

        std::string directory; // Directory, where template nodes are stored
        std::string nodeNamePattern; // Name pattern for sleeper variants
        int variantsCount{ 0 }; // Sleeper variants template nodes, stored in 'directory', count
        bool addRandomAngle{true}; // Should the random angle be added
        double randomAngle{0.0}; // Random angle to add to sleeper rotation in Z axis

        MaterialInfo defaultMaterialInfo;
    };

    /**
     * Paramers for point rails generation
     */
    struct PointRailGenerationData
    {
    public:
        /**
         * Choose nearest available pointRail length in millimeters.
         */
        int chooseSuitablePointRailLength(int availableSpaceMM, bool leftSide) const;
        std::string getPointRailPath(int lengthMM, bool leftSide) const;

        void loadFromProto(const proto::meshgen::PointRail& source);
        void saveToProto(proto::meshgen::PointRail& source) const;

        std::string directory;
        std::string nodeNamePattern;

        int stockOffset{ 150 };
        int xMiddleGap{ 400 };
        int xNarrowSharpFrogMaxOffset{4000};

        MaterialInfo materialInfo;

    private:
        /**
         * Scan 'directory' for files matching 'nodeNamePattern' and fill 'm_leftAvailableLengthsMM'
         * and 'm_rightAvailableLengthsMM'.
         * Return 'false' if 'directory' or 'nodeNamePattern' have invalid value, 'true' otherwise.
         */
        bool loadAvailableLengths();

        std::vector<int> m_leftAvailableLengthsMM;
        std::vector<int> m_rightAvailableLengthsMM;
    };

    /**
     * Paramers for switch rods generation
     */
    struct SwitchRodGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::SwitchRods& source);
        void saveToProto(proto::meshgen::SwitchRods& source) const;

        const std::string& getHeadRodNodePath(std::optional<bool> forceLeft = std::nullopt) const;

        struct SwitchBoxNodeInfo
        {
            bool isLeft;
            std::string nodePath;
        };
        SwitchBoxNodeInfo getSwitchBoxNodeInfo(std::optional<bool> forceLeft = std::nullopt) const;

        std::string rodNodePath;
        std::string headRodNodePathLeft;
        std::string headRodNodePathRight;
        std::string switchBoxNodePathLeft;
        std::string switchBoxNodePathRight;
        bool switchBoxPreferLeft{false};
        std::string borderBarPath;
    };

    /**
     * Paramers for guard rails generation
     */
    struct GuardRailsGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::GuardRails& source);
        void saveToProto(proto::meshgen::GuardRails& source) const;

        std::string directory;
        std::string leftNode;
        std::string rightNode;
        std::string middleNode;
        std::string mountNode;

        double visibleDistance{ 50.0 };
        int offset{ 0 };
        int startGap{ 250 };
        int endGap{ 120 };
    };

    /**
     * Paramers for rail frogs generation
     */
    struct RailFrogGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::RailFrog& source);
        void saveToProto(proto::meshgen::RailFrog& source) const;

        std::string directory;
        int startGap{ 150 };
        int endGap{ 150 };
        int mountsOffset{ 150 };
        int mountsOffsetObtuse{ 75 };
        bool skinned{false};

        MaterialInfo materialInfo;
    };

    /**
     * Paramers for heel block generation
     */
    struct HeelBlockGenerationData
    {
    public:
        void loadFromProto(const proto::meshgen::HeelBlock& source);
        void saveToProto(proto::meshgen::HeelBlock& source) const;

        std::string directory;

        std::string leftInnerPart;
        std::string rightInnerPart;
        std::string middlePart;
        std::string outerPart;

        MaterialInfo materialInfo;

        bool flipLeftInnerPart{ false };
        bool flipRightInnerPart{ false };

        double halfSize{0.4};
    };

    /**
     * Adjunct part config entry.
     * @see http://git.neokon.local:22580/root/GlobalTerrainEditor-release/wikis/Control-parameters-of-mesh-generation-for-adjuncts
     * wiki page for details, Item 1. 
     */
    struct AdjunctPartData
    {
    public:
        void loadFromProto(const proto::meshgen::AdjunctPart& source,
            const std::vector<SurfaceGenerationDataPtr>& namedSurfaces);
        void saveToProto(proto::meshgen::AdjunctPart& source) const;

        bool enabled{true};
        std::string name;

        std::vector<SurfaceGenerationDataPtr> surfaces;

        int32_t frontDistanceOffsetMm{0};
        int32_t backDistanceOffsetMm{0};

        int32_t baselineOffsetHeightMm{0};
        int32_t baselineOffsetSideMm{0};

        bool enableBraces{false};
        std::string braceNodePath;
        int32_t braceIntervalMm{0};
        int32_t braceGapMm{0};
        int32_t firstBraceDistanceOffsetMm{0};
        int32_t secondBraceDistanceOffsetMm{0};
        int32_t penultBraceDistanceOffsetMm{0};
        int32_t lastBraceDistanceOffsetMm{0};
        bool rotateBraces{false};
        bool fitBracesToSleepers{false};
        int32_t braceSleeperOffsetMm{0};

        bool enableFrontCap{false};
        std::string frontCapNodePath;

        bool enableBackCap{false};
        std::string backCapNodePath;
    };

    struct AdjunctData
    {
    public:
        void loadFromProto(const proto::meshgen::Adjunct& source,
            const std::vector<SurfaceGenerationDataPtr>& namedSurfaces);
        void saveToProto(proto::meshgen::Adjunct& source) const;

        std::vector<AdjunctPartData> parts;
    };

    struct BranchGuardRailData
    {
    public:
        void loadFromProto(proto::meshgen::BranchGuardRail const& source);
        void saveToProto(proto::meshgen::BranchGuardRail source) const;

        std::string fishPlateNode;
        MaterialInfo fishPlateMaterialInfo;

        std::string mountNode;

        int railGapMm{ 200 };
        int endPartLengthMm{ 3000 };
    };

public:
    RailwayMeshGeneratorConfig();

    bool loadFromProto(const proto::meshgen::Config& source);
    bool fillPropertiesFromJsonFile(const std::string& meshGenConfigFile);
    bool fillPropertiesFromJsonFiles(const std::string& files);
    bool savePropertiesToJsonFile(const std::string& meshGenConfigFile);

    static double toInnerUnits(int value) { return value * toInnerUnitsScale; }
    static int toConfigUnits(double value) { return static_cast<int>(value * toConfigUnitsScale); }

    std::vector<SurfaceGenerationDataUnique>& getSurfacesData() { return m_surfaceGenData; }
    SurfaceGenerationData& getSurfaceData(int index);
    const JunctionSleepersGenerationData& getJunctionSleepersData() const {
        return m_junctionSleepersGenData; }
    const BranchSleepersGenerationData& getBranchSleepersData() const { 
        return m_branchSleepersGenData; }
    const PointRailGenerationData& getPointRailsData() const { return m_pointRailsGenData; }
    const SwitchRodGenerationData& getSwitchRodsData(int type = +SwitchRodsType::Default) const {
        return m_switchRodsGenData[type]; }
    const GuardRailsGenerationData& getGuardRailsData() const { return m_guardRailsGenData; }
    const RailFrogGenerationData& getRailFrogData() const { return m_railFrogGenData; }
    const HeelBlockGenerationData& getHeelBlockData() const { return m_heelBlockData; }
    const BranchCommonGenerationData& getBranchCommonData() const { return m_branchCommonGenData; }
    const CommonGenerationData& getCommonData() const { return m_commonGenData; }
    const AdjunctData& getAdjunctData() const { return m_adjunctData; }
    const BranchGuardRailData& getBranchGuardRailData() const { return m_branchGuardRailData; }

    bool isOptionAllowed(const std::string& optName,
        const std::vector<std::string>& contextsNames) const;

private:
    std::vector<SurfaceGenerationDataUnique> m_surfaceGenData;

    JunctionSleepersGenerationData m_junctionSleepersGenData;
    BranchSleepersGenerationData m_branchSleepersGenData;
    PointRailGenerationData m_pointRailsGenData;
    std::array<SwitchRodGenerationData, +SwitchRodsType::Total> m_switchRodsGenData;
    GuardRailsGenerationData m_guardRailsGenData;
    RailFrogGenerationData m_railFrogGenData;
    HeelBlockGenerationData m_heelBlockData;
    CommonGenerationData m_commonGenData;
    BranchCommonGenerationData m_branchCommonGenData;
    AdjunctData m_adjunctData;
    BranchGuardRailData m_branchGuardRailData;

    std::map<std::string, std::vector<std::string>> m_allowedOptions;

    std::vector<PointProfileDataPtr> m_pointProfiles;
    std::vector<SurfaceGenerationDataPtr> m_namedSurfaces;
};

template<typename ObjType>
bool RailwayMeshGeneratorConfig::SurfaceGenerationData::applySurfaceParams(const ObjType& object,
    const char* surfaceName, const MaterialInfo& addMaterialParams) const
{
    int index = object->findSurface(surfaceName);
    if (index < 0)
    {
        return false;
    }
    
    object->setPhysicsFriction(1.0, index);
    object->setPhysicsRestitution(1.0, index);
    object->setMinVisibleDistance(minVisibleDistance, index);
    object->setMaxVisibleDistance(maxVisibleDistance, index);
    object->setMinFadeDistance(minFadeDistance, index);
    object->setMaxFadeDistance(maxFadeDistance, index);
    object->setIntersection(true, index);
    object->setIntersectionMask(+IntersectionMasks::Railways, index);

    object->setCollisionMask(collisionMask, index);
    if (collisionMask != 0)
    {
        object->setCollision(1, index);
    }

    auto foundIt = customMaterials.find(surfaceName);
    if (foundIt != customMaterials.cend())
    {
        object->setMaterial(foundIt->second.data(), index);
        return true;
    }

    auto tmpMat = defaultMaterialInfo.getOrCreateMaterial(addMaterialParams);
    return tmpMat ? (object->setMaterial(tmpMat, index) != 0) : false;
}
