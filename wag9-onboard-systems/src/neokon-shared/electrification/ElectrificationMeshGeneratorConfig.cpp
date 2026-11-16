#include "ElectrificationMeshGeneratorConfig.h"

#include "neokon-shared/JsonStreamToProtoMessage.h"
#include "neokon-shared/ProtoMessageToJsonStream.h"
#include "neokon-shared/ProtobufUnigineInterop.h"

#include <UnigineEngine.h>

#include <iostream>
#include <fstream>

namespace proto_preset = proto::electrification::preset;
using namespace electrification;

//***********************************************************
// ElectrificationMeshGeneratorConfig implementation.
//***********************************************************
bool
ElectrificationMeshGeneratorConfig::fillPropertiesFromJsonFile(const std::string& meshGenConfigFile)
{
    proto_preset::ElectrificationPreset presetProto;

    std::ifstream stream(meshGenConfigFile);
    if (!stream)
    {
        return false;
    }

    try
    {
        jsonStreamToProtoMessage(stream, presetProto);
    }
    catch (std::exception const& err)
    {
        Unigine::Log::error("Failed to load \"%s\": %s\n", meshGenConfigFile.data(), err.what());
        stream.close();
        return false;
    }

    if (presetProto.has_common_params())
    {
        m_commonParamsData.loadFromProto(presetProto.common_params());
    }

    if (presetProto.has_catenary())
    {
        m_catenaryData.loadFromProto(presetProto.catenary());
    }

    if (presetProto.has_pole())
    {
        m_poleData.loadFromProto(presetProto.pole());
    }

    if (presetProto.has_portal())
    {
        m_portalData.loadFromProto(presetProto.portal());
    }

    if (presetProto.has_portal_suspension())
    {
        m_portalSuspensionData.loadFromProto(presetProto.portal_suspension());
    }

    if (presetProto.has_anchor())
    {
        m_anchorData.loadFromProto(presetProto.anchor());
    }

    return true;
}

bool
ElectrificationMeshGeneratorConfig::savePropertiesToJsonFile(const std::string& meshGenConfigFile)
{
    proto_preset::ElectrificationPreset presetProto;

    m_commonParamsData.saveToProto(*presetProto.mutable_common_params());
    m_catenaryData.saveToProto(*presetProto.mutable_catenary());
    m_poleData.saveToProto(*presetProto.mutable_pole());
    m_portalData.saveToProto(*presetProto.mutable_portal());
    m_portalSuspensionData.saveToProto(*presetProto.mutable_portal_suspension());
    m_anchorData.saveToProto(*presetProto.mutable_anchor());

    std::ofstream stream(meshGenConfigFile);
    if (!stream)
    {
        return false;
    }

    try
    {
        protoMessageToJsonStream(presetProto, stream);
    }
    catch (std::exception const&)
    {
        stream.close();
        return false;
    }

    return true;
}

//***************************************************************************************
// CommonParamsData implementaion.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::CommonParamsData::loadFromProto(
    proto_preset::CommonParams const& dataBlock)
{
    projectCode = projectCodeFromProto(dataBlock.project_code());
    catenaryWireType = catenaryWireTypeFromProto(dataBlock.catenary_type_idx());
    consoleProfileCut = consoleProfileCutFromProto(dataBlock.console_profile_cut());
}

void ElectrificationMeshGeneratorConfig::CommonParamsData::saveToProto(
    proto_preset::CommonParams& dataBlock) const
{
    dataBlock.set_project_code(projectCodeToProto(projectCode));
    dataBlock.set_catenary_type_idx(catenaryWireTypeToProto(catenaryWireType));
    dataBlock.set_console_profile_cut(consoleProfileCutToProto(consoleProfileCut));
}

//***************************************************************************************
// SpanwireData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::SpanwireData::loadFromProto(
    proto_preset::Spanwire const& dataBlock)
{
    if (dataBlock.has_spanwire_insulator())
    {
        spanwireInsulator.loadFromProto(dataBlock.spanwire_insulator());
    }

    insulatorPlacement = spanwireInsulatorPlacementFromProto(dataBlock.insulator_placement());
}

void ElectrificationMeshGeneratorConfig::SpanwireData::saveToProto(
    proto_preset::Spanwire& dataBlock) const
{
    spanwireInsulator.saveToProto(*dataBlock.mutable_spanwire_insulator());
    dataBlock.set_insulator_placement(spanwireInsulatorPlacementToProto(insulatorPlacement));
}

//***************************************************************************************
// InsulatorData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::InsulatorData::loadFromProto(
    proto_preset::Insulator const& dataBlock)
{
    type = insulatorTypeFromProto(dataBlock.type());
    construct = insulatorConstructFromProto(dataBlock.construct());
    material = insulatorMaterialFromProto(dataBlock.material());
    allowedVoltage = dataBlock.allowed_voltage_kv();
}

void ElectrificationMeshGeneratorConfig::InsulatorData::saveToProto(
    proto_preset::Insulator& dataBlock) const
{
    dataBlock.set_type(insulatorTypeToProto(type));
    dataBlock.set_construct(insulatorConstructToProto(construct));
    dataBlock.set_material(insulatorMaterialToProto(material));
    dataBlock.set_allowed_voltage_kv(allowedVoltage);
}

//***************************************************************************************
// AnchorData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::AnchorData::loadFromProto(
    proto_preset::Anchor const& dataBlock)
{
    if (dataBlock.has_tension_insulator())
    {
        tensionInsulator.loadFromProto(dataBlock.tension_insulator());
    }

    backTieNode = dataBlock.back_tie_node();
    fixedAnchorBlockNodeName
        = dataBlock.fixed_anchor_block_node_name();
    compensatedAnchorBlockNodeName
        = dataBlock.compensated_anchor_block_node_name();
    semiCompensatedAnchorBlockNodeName
        = dataBlock.semi_compensated_anchor_block_node_name();
}

void ElectrificationMeshGeneratorConfig::AnchorData::saveToProto(
    proto_preset::Anchor& dataBlock) const
{
    tensionInsulator.saveToProto(*dataBlock.mutable_tension_insulator());
    dataBlock.set_back_tie_node(backTieNode);
    dataBlock
        .set_fixed_anchor_block_node_name(fixedAnchorBlockNodeName);
    dataBlock
        .set_compensated_anchor_block_node_name(
            compensatedAnchorBlockNodeName);
    dataBlock
        .set_semi_compensated_anchor_block_node_name(
            semiCompensatedAnchorBlockNodeName);
}

//***************************************************************************************
// PortalSuspensionData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::PortalSuspensionData::loadFromProto(
    proto_preset::PortalSuspension const& dataBlock)
{
    nodeName = dataBlock.nodename();
}

void ElectrificationMeshGeneratorConfig::PortalSuspensionData::saveToProto(
    proto_preset::PortalSuspension& dataBlock) const
{
    dataBlock.set_nodename(nodeName);
}

//***************************************************************************************
// PortalData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::PortalData::loadFromProto(
    proto_preset::Portal const& dataBlock)
{
    blockType = portalBlockTypeFromProto(dataBlock.block_type());
    blockMaterial = portalBlockMaterialFromProto(dataBlock.block_material());
    pillarMaterial = pillarMaterialFromProto(dataBlock.pillar_material());
    leftPillarType = pillarTypeFromProto(dataBlock.left_pillar_type());
    rightPillarType = pillarTypeFromProto(dataBlock.right_pillar_type());
    nodePrefixFoundationColor = dataBlock.node_prefix_foundation_color();
    nodePrefixHeight = dataBlock.node_prefix_height();

    if (dataBlock.has_spanwire())
    {
        SpanwireData spanwire_;
        spanwire_.loadFromProto(dataBlock.spanwire());

        spanwire = spanwire_;
    }
}

void ElectrificationMeshGeneratorConfig::PortalData::saveToProto(
    proto_preset::Portal& dataBlock) const
{
    dataBlock.set_block_type(portalBlockTypeToProto(blockType));
    dataBlock.set_block_material(portalBlockMaterialToProto(blockMaterial));
    dataBlock.set_pillar_material(pillarMaterialToProto(pillarMaterial));
    dataBlock.set_left_pillar_type(pillarTypeToProto(leftPillarType));
    dataBlock.set_right_pillar_type(pillarTypeToProto(rightPillarType));
    dataBlock.set_node_prefix_height(nodePrefixHeight);
    dataBlock.set_node_prefix_foundation_color(nodePrefixFoundationColor);

    if (spanwire)
    {
        spanwire->saveToProto(*dataBlock.mutable_spanwire());
    }
}

//***************************************************************************************
// PoleData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::PoleData::loadFromProto(
    proto_preset::Pole const& dataBlock)
{
    if (dataBlock.has_cantilever_insulator())
    {
        cantileverInsulator.loadFromProto(dataBlock.cantilever_insulator());
    }

    pillarMaterial = pillarMaterialFromProto(dataBlock.material());
    nodePrefixHeight = dataBlock.node_prefix_height();
    nodePrefixFoundationColor = dataBlock.node_prefix_foundation_color();
}

void ElectrificationMeshGeneratorConfig::PoleData::saveToProto(
    proto_preset::Pole& dataBlock) const
{
    cantileverInsulator.saveToProto(
        *dataBlock.mutable_cantilever_insulator());
    dataBlock.set_material(pillarMaterialToProto(pillarMaterial));
    dataBlock.set_node_prefix_height(nodePrefixHeight);
    dataBlock.set_node_prefix_foundation_color(nodePrefixFoundationColor);
}

//***************************************************************************************
// CatenaryData implementation.
//***************************************************************************************
void ElectrificationMeshGeneratorConfig::CatenaryData::loadFromProto(
    proto_preset::Catenary const& dataBlock)
{
    if (dataBlock.has_tension_insulator())
    {
        InsulatorData data;
        data.loadFromProto(dataBlock.tension_insulator());
        tensionInsulator = data;
    }

    if (dataBlock.has_section_insulator())
    {
        InsulatorData data;
        data.loadFromProto(dataBlock.section_insulator());
        sectionInsulator = data;
    }

    wireDiameter = dataBlock.wire_diameter();
    cableDiameter = dataBlock.cable_diameter();
    dropperDiameter = dataBlock.dropper_diameter();
}

void ElectrificationMeshGeneratorConfig::CatenaryData::saveToProto(
    proto_preset::Catenary& dataBlock) const
{
    if (tensionInsulator)
    {
        tensionInsulator->saveToProto(*dataBlock.mutable_tension_insulator());
    }

    if (sectionInsulator)
    {
        sectionInsulator->saveToProto(*dataBlock.mutable_section_insulator());
    }

    dataBlock.set_wire_diameter(wireDiameter);
    dataBlock.set_cable_diameter(cableDiameter);
    dataBlock.set_dropper_diameter(dropperDiameter);
}
