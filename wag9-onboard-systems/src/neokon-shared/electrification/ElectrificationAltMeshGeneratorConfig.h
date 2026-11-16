#pragma once

#include "ElectrificationPresetEnumTypes.h"
#include "ElectrificationSetEnumTypes.h"

#include <memory>
#include <optional>
#include <string>

class ElectrificationAltMeshGeneratorConfig;
using ElectrificationMeshGeneratorConfigPtr =
    std::shared_ptr<ElectrificationAltMeshGeneratorConfig>;
using ElectrificationMeshGeneratorConfigLink = std::weak_ptr<ElectrificationAltMeshGeneratorConfig>;

/**
 * Данный класс загружает пресеты - конфигурационные файлы для динамической генерации контента,
 * элементов контактной сети железных дорог (электрификации).Загружаемый формат пресетов описан
 * в следующих скриптах:
 *
 * -neokon-shared/proto/electrification/ElectrificationAltPreset.proto
 * -neokon-shared/proto/electrification/ElectrificationAltPresetAnchor.proto
 * -neokon-shared/proto/electrification/ElectrificationAltPresetCatenary.proto
 * -neokon-shared/proto/electrification/ElectrificationAltPresetCommonParams.proto
 * -neokon-shared/proto/electrification/ElectrificationAltPresetPole.proto
 * -neokon-shared/proto/electrification/ElectrificationAltPresetPortalSuspension.proto
 * -neokon-shared/proto/electrification/ElectrificationAltPresetPortal.proto
 */
class ElectrificationAltMeshGeneratorConfig
{
public:
    struct CatenaryAttachableNode
    {
    public:
        void loadFromProto(
            proto::electrification::alternative_preset::AttachableNode const& dataBlock);
        void saveToProto(
            proto::electrification::alternative_preset::AttachableNode& dataBlock);

        std::string shortNode;
        std::string mediumNode;
        std::string longNode;
    };

    struct CatenaryWireInsulators
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::WireInsulators const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::WireInsulators& dataBlock);

        std::string nodeUpper;
        std::string nodeLower;
        double distance{ 0.0 };
        bool useDroppers;
    };

    struct DoublePoleNodes
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::Pole_DoublePoleNodes const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::Pole_DoublePoleNodes& dataBlock);

        std::string poleDoubleNodePath;
        std::string poleA;
        std::string poleB;
        std::string poleC;
        std::string poleInsA;
        std::string poleInsB;
        std::string poleInsC;
        std::string poleJunctionL;
        std::string poleJunctionR;
    };

    struct DoubleSuspensionNodes
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::PortalSuspension_DoubleSuspensionNodes const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::PortalSuspension_DoubleSuspensionNodes& dataBlock);

        std::string spnDoubleNodePath;
        std::string spnA;
        std::string spnB;
        std::string spnC;
        std::string spnInsA;
        std::string spnInsB;
        std::string spnInsC;
        std::string spnJunctionL;
        std::string spnJunctionR;
    };

    struct CommonParamsData
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::CommonParams const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::CommonParams& dataBlock);

        electrification::CatenaryType catenaryType{ electrification::CatenaryType::Simple };
        electrification::ContactWireType contactWireType{ electrification::ContactWireType::T2W };
        electrification::TensioningType tensioningType{ electrification::TensioningType::None };
        std::string contactWireTypeStr;
        std::string additionalContactWireTypeStr;

        int overlapSpans{ 0 };
    };

    struct CatenaryData
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::Catenary const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::Catenary& dataBlock);

        CatenaryAttachableNode dropperNode;
        CatenaryAttachableNode insulatorDropperNode;
        CatenaryAttachableNode compMidPointAnchorNode;
        CatenaryAttachableNode scompMidPointAnchorNode;

        CatenaryWireInsulators tensionInsulator;
        CatenaryWireInsulators sectionInsulator;
        CatenaryWireInsulators overlapInsulator;

        std::string wireMaterial;
        double contactWireDiameter{ 0.0 };
        double catenaryWireDiameter{ 0.0 };
        double dropperDiameter{ 0.0 };
        double saggingCoefficient{ 1.0 };
        double maxVisibilityDistance{ 50.0 };
        double maxFadeDistance{ 0.0 };
        double overlapInsulatorDtopperStepM{1.0};
        bool useMeshDropper{ false };
        int numOverlapInsulatorDroppers{ 0 };
    };

    struct PoleData
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::Pole const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::Pole& dataBlock);

        DoublePoleNodes poleDoubleNode;
        std::string poleNode;
    };

    struct PortalSuspensionData
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::PortalSuspension const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::PortalSuspension& dataBlock);

        DoubleSuspensionNodes spnDoubleNode;
        std::string spnNode;
        double spnZShiftM{ 0.0 };
    };

    struct PortalData
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::Portal const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::Portal& dataBlock);

        std::string leftPoleNode;
        std::string rightPoleNode;
        std::string crossBarNode;
        std::string spanWireInsulatorNode;
        std::string crossWireInsulatorNode;
        std::string dropperClamp;

        double leftPortalPoleGap{0.0};
        double rightPortalPoleGap{0.0};
        double crossBarZShiftM{ 0 };
        double spanWireDiameter{0.0};
        double spanWireInsulatorTrackDistance{4.0};
        double spanWireInsulatorDoubleDistance{7.0};
        double spanWireInsulatorOffset{2.0};
        double crossWireSaggingCoefficient{1.0};
        double spanWireConnectorOffset{1.0};

        bool allowSpanWire{ false };
        bool isCrossSpan{ false };

        CatenaryAttachableNode spanWireConnector;
    };

    struct AnchorData
    {
    public:
        void loadFromProto(proto::electrification::alternative_preset::Anchor const& dataBlock);
        void saveToProto(proto::electrification::alternative_preset::Anchor& dataBlock);

        std::string fixedAnchorBlockNode;
        std::string fixedBackTieNode;
        std::string compensatedAnchorBlockNode;
        std::string compensatedBackTieNode;
        std::string semiCompensatedAnchorBlockNode;
        std::string semiCompensatedBackTieNode;
        std::string midAnchorNode;
        std::string midBackTie;
    };

    struct CustomNodeDataList
    {
    public:
        void loadFromProto(
            proto::electrification::alternative_preset::ElectrificationAlternativePreset const&
                dataBlock);
        void saveToProto(
            proto::electrification::alternative_preset::ElectrificationAlternativePreset&
                dataBlock);

        std::map<std::string, std::string> nodePathsByNames;
    };

public:
    bool fillPropertiesFromJsonFile(const std::string& meshGenConfigFile);
    bool savePropertiesToJsonFile(const std::string& meshGenConfigFile);

    const CommonParamsData& getCommonParamsData() const { return m_commonParamsData; }
    const CatenaryData& getCatenaryData() const { return m_catenaryData; }
    const PoleData& getPoleData() const { return m_poleData; }
    const PortalSuspensionData& getPortalSuspensionData() const { return m_portalSuspensionData; }
    const PortalData& getPortalData() const { return m_portalData; }
    const AnchorData& getAnchorData() const { return m_anchorData; }
    const CustomNodeDataList& getCustomNodeData() const { return m_customNodeDataList; }

private:
    CommonParamsData m_commonParamsData;
    CatenaryData m_catenaryData;
    PoleData m_poleData;
    PortalSuspensionData m_portalSuspensionData;
    PortalData m_portalData;
    AnchorData m_anchorData;
    CustomNodeDataList m_customNodeDataList;

    std::map<std::string, std::string> m_customPoleNodesPathsByNames;
};
