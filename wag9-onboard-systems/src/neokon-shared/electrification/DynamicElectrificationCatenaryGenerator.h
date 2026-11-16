#pragma once

#include "neokon-shared/BezierSegment.h"
#include "neokon-shared/electrification/DynamicElectrificationPart.h"
#include "neokon-shared/electrification/ElectrificationAltMeshGeneratorConfig.h"
#include "neokon-shared/electrification/ElectrificationContextDataTypes.h"
#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"
#include "neokon-shared/electrification/utils/ElectrificationPoleHelperData.h"
#include "neokon-shared/railways/ObjectMessageLogger.h"

#include <Unigine.h>

#include <boost/icl/split_interval_map.hpp>

class Wire;
using WirePtr = std::shared_ptr<Wire>;

class MorticeAttachable;
using MorticeAttachablePtr = std::shared_ptr<MorticeAttachable>;

class Insulator;
using InsulatorPtr = std::shared_ptr<Insulator>;

class DropperClamp;
using DropperClampPtr = std::shared_ptr<DropperClamp>;

enum class MidPointAnchorType
{
    NONE,
    MIDDLE,
    START,
    END
};

/**
 * Generator class for dynamic catenary.
 * Creates catenary segment mesh for one segment of elecrification tension.
 * Dependent on DynamicElectrificationPoleGenerator: all parameters are set up in there.
 * isDirrefentWith() method is defined by DynamicElectrificationPoleGenerator.
 * One catenary segment can contain one messenger wire (несущий трос)
 * which holds any number of contact wires (контактных проводов).
 * Stitch catenary shorter than ELECTRIFICATION_STITCHED_CATENARY_LENGTH_MIN
 * automatically turns into non-stitched.
 */
class DynamicElectrificationCatenaryGenerator : public DynamicElectrificationPart
{
public:
    struct CreateArgsBase
    {
        electrification::CreationMode creationMode{ electrification::CreationMode::CLUSTER };
        std::string defaultCantileverPreset;

        MidPointAnchorType midPointAnchorType{MidPointAnchorType::NONE};
    };

    struct CreateArgs : CreateArgsBase
    {
        electrification::CatenaryContextData contextData;
        /**
         * Is catenary object represents span wire.
         * Span wire is a wire between portal registration arms.
         * It contains special insulators between portal suspensions.
         */
        bool isSpanWire{ false };
        bool isCrossSpan{ false };
        bool insulatedOverlap{ false };
        /**
         * Whether the wa/wn connector is placed at the end of the overlap instead of start.
         */
        bool isInsulatedOverlapFlipped{ false };
        std::set<double> spanWireSuspensionDistances;
        std::vector<Unigine::Math::Vec3> dropperPositions;
    };

    struct ConnectTwoPolesByCatenaryArgs : CreateArgsBase
    {
        ConnectTwoPolesByCatenaryArgs(electrification::PoleContextData const& leftPole,
            electrification::PoleContextData const& rightPole, ObjectMessageLogger* log);

        electrification::PoleContextData const& leftPole;
        electrification::PoleContextData const& rightPole;
        ObjectMessageLogger* log{ nullptr }; //< Not used if nullptr.

        std::string leftPolePreset;
        std::string rightPolePreset;
        bool leftStitch{ false };
        bool rightStitch{ false };
        electrification::CreationMode creationMode{ electrification::CreationMode::CLUSTER };
    };

    struct ConnectTwoCantileversArgs
    {
        electrification::PoleCantileverConnectors leftCantilever;
        electrification::PoleCantileverConnectors rightCantilever;
        Unigine::Math::Mat4 leftTransform;
        Unigine::Math::Mat4 rightTransform;
        bool leftStitch{ false };
        bool rightStitch{ false };
    };

    struct AttachableInfo
    {
        electrification::CatenaryAttachableType type;
        electrification::CatenaryAttachableLength lengthType;
        Unigine::Math::Mat4 transform;
    };

    /*struct InsulatorInfo
    {
        std::string nodeName;
        double distance{0.0};
        bool useDroppers{false};

        std::optional<Unigine::Math::Vec3> ntIn;
        std::optional<Unigine::Math::Vec3> ntOut;
        std::optional<Unigine::Math::Vec3> kpNearIn;
        std::optional<Unigine::Math::Vec3> kpNearOut;
        std::optional<Unigine::Math::Vec3> kpFarIn;
        std::optional<Unigine::Math::Vec3> kpFarOut;
    };*/

    static std::list<CreateArgs> connectTwoPolesByCatenary(
        ConnectTwoPolesByCatenaryArgs const& args);
    static electrification::CatenaryContextData connectTwoCantileversByCatenary(
        ConnectTwoCantileversArgs const& args);

public:
    DynamicElectrificationCatenaryGenerator(uint32_t id, CreateArgs&& args);
    virtual bool load() override;
    virtual void unload() override;
    virtual Unigine::NodePtr createNodes() override;
    virtual void validateClusters(GetOrCreateClusterFunc const& getOrCreateCluster,
        std::optional<Unigine::Math::Vec3> const& center = std::nullopt) const override;
    /**
     * @return global transform for catenary segment.
     */
    Unigine::Math::Mat4 globalTransform() const;

    Unigine::NodePtr rootNode() const;

private:
    using NodeLengthMap = std::map<electrification::CatenaryAttachableLength, double>;
    NodeLengthMap getNodeLengths(electrification::CatenaryAttachableNodeName const& nodeNames);
    std::optional<electrification::CatenaryAttachableLength> chooseNodeLengthType(
        NodeLengthMap const& nodeLengths, double distance);

private:
    /**
     * Utility method for createNodes() and validateCluster().
     */
    void addNodes( 
        std::function<void(std::string const&, Unigine::Math::Mat4 const&)>& addNodeFunc) const;
    /**
     * Generate or load droppers on positions set by getDropperParams() method.
     */
    void generateDroppers();
    /**
     * Generate or load droppers on positions set in m_genArgs.dropperPositions.
     */
    void calcDroppersFromPositions();
    /**
     * Generate or load insulator droppers in set positions.
     */
    void generateInsulatorDroppers(MorticeAttachablePtr insulator);
    /**
     * Creates elements of stitched catenary.
     */
    void generateStitchWires();
    /**
     * Adds string mesh from @startPos to @endPos,
     * optionally can add knot mesh at the end of string.
     */
    void generateStraightWire(Unigine::Math::Vec3 const& startPos,
        Unigine::Math::Vec3 const& endPos, bool addConnector, std::string const& surfaceName);
    /**
     * Generates mesh connector.
     */
    void generateWireConnector(Unigine::Math::Vec3 const& startPos,
        Unigine::Math::Vec3 const& endPos, std::string const& surfaceName);
    /**
     * Adds mid point anchor for compensated and semicompensated catenary.
     */
    void generateMidPointAnchor();

    /**
     * Adds tft connector offseted from the most middle dropper.
     */
    void generateSpanWireConnector();

    /**
     * Adds all mesh surfaces from @meshToAdd to @m_mesh.
     */
    void addMeshPart(Unigine::MeshPtr const& meshToAdd, std::string const& surfaceName);

    /**
     * Creates Wire object for messenger wire, inserts all insulators if necessary.
     */
    void generateMessengerWire();
    /**
     * Creates Wire object for contact wire, inserts all insulators if necessary.
     */
    void generateContactWires();
    /**
     * Creates all insulators for registration arm wire depending on
     * m_genParams.spanWireSuspensionDistances.
     */
    void generateSpanWireInsulators();

    /**
     * @return catenary-dependent transform for vector that goes vertically down from @point.
     */
    Unigine::Math::mat4 getVerticalTransform(Unigine::Math::Vec3 point) const;

    /**
     * Get a position with set tParam (relative to messenger wire) right between all contact wires.
     * If the number of contact wires is 1,
     * method is equal to eval(tParam) of the only contact wire.
     */
    std::optional<Unigine::Math::Vec3> getAverageContactWirePos(double tParam);

    /**
     * The vertical distance between cable maximum and minimum height above the ground.
     */
    double maxOffset() const;
    /**
     * @return the list of parameters for cable's bezier segment where droppers should be placed.
     */
    std::vector<double> getDropperParams();
    /**
     * @return the list of parameters for cable's bezier segment where droppers should be placed
     * around insulator for insulated overlap.
     */
    std::vector<double> getInsulatedOverlapDropperParams();

    /**
     * @return global position of catenary.
     */
    Unigine::Math::Vec3 getTranslate() const;
    /**
     * The local transform of catenary segment is codirectional to Math::Vec3::FORWARD vector.
     */
    Unigine::Math::Mat4 toLocalTransform() const;

    /**
     * Creates node or mesh dropper from @messengerPos to @contactPos.
     * If @force1w is true, use 1w node instead of 2w.
     */
    void addDropper(int dropperId, Unigine::Math::Vec3 const& messengerPos,
        Unigine::Math::Vec3 const& contactPos,
        bool force1w, NodeLengthMap const& lengthMap);

    void maybeCreateRootNode();

    CreateArgs m_genParams;

    double m_contactRadius{ 0.0 };
    double m_messengerRadius{ 0.0 };
    double m_dropperRadius{ 0.0 };
    double m_saggingCoefficient{ 1.0 };
    double m_insulatorDropperStepM{ 1.0 };
    double m_maxDropperWidth{0.0};
    double m_spanConnectorOffset{0.0};
    int m_numInsulatorDroppers{ 0 };
    bool m_useMeshDropper{false};
    std::string m_dropperClampNodeName;

    Unigine::MeshPtr m_mesh;

    WirePtr m_messengerWire;
    std::map<electrification::ConnectorId, WirePtr> m_contactWires;
    int m_mainWireId{-1};

    std::vector<double> m_dropperParams;
    std::map<int, bool> m_dropperNodeLoadable;
    std::list<AttachableInfo> m_attachableTransforms;
    boost::icl::split_interval_set<double> m_attachablesIntervals;
    std::unordered_multimap<electrification::InsulatorPositionType, InsulatorPtr>
        m_insulatorsByType;

    Unigine::NodePtr m_rootNode;
};

using DynamicElectrificationCatenaryGeneratorPtr = std::shared_ptr<DynamicElectrificationCatenaryGenerator>;
