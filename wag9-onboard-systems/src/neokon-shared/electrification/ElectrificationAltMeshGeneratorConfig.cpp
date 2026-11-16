#include "ElectrificationAltMeshGeneratorConfig.h"

#include "neokon-shared/JsonStreamToProtoMessage.h"
#include "neokon-shared/ProtoMessageToJsonStream.h"
#include "neokon-shared/ProtobufUnigineInterop.h"

#include <UnigineEngine.h>

#include <unordered_map>
#include <iostream>
#include <fstream>
#include <regex>

namespace altpreset = proto::electrification::alternative_preset;

/////////////////////////////////////////////////////////////////////////////////////////
// class ElectrificationAltMeshGeneratorConfig
/////////////////////////////////////////////////////////////////////////////////////////
bool
ElectrificationAltMeshGeneratorConfig::fillPropertiesFromJsonFile(
    const std::string& meshGenConfigFile)
{
    altpreset::ElectrificationAlternativePreset presetProto;

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

    if (presetProto.has_portal_suspension())
    {
        m_portalSuspensionData.loadFromProto(presetProto.portal_suspension());
    }

    if (presetProto.has_portal())
    {
        m_portalData.loadFromProto(presetProto.portal());
    }

    if (presetProto.has_anchor())
    {
        m_anchorData.loadFromProto(presetProto.anchor());
    }

    m_customNodeDataList.loadFromProto(presetProto);

    return true;
}

bool ElectrificationAltMeshGeneratorConfig::savePropertiesToJsonFile(
    const std::string& meshGenConfigFile)
{
    altpreset::ElectrificationAlternativePreset presetProto;

    m_commonParamsData.saveToProto(*presetProto.mutable_common_params());
    m_catenaryData.saveToProto(*presetProto.mutable_catenary());
    m_poleData.saveToProto(*presetProto.mutable_pole());
    m_portalSuspensionData.saveToProto(*presetProto.mutable_portal_suspension());
    m_portalData.saveToProto(*presetProto.mutable_portal());
    m_anchorData.saveToProto(*presetProto.mutable_anchor());
    m_customNodeDataList.saveToProto(presetProto);

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

//=======================================================================================
// Primary data blocks
//=======================================================================================

/////////////////////////////////////////////////////////////////////////////////////////
// struct CommonParamsData
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::CommonParamsData::loadFromProto(
    altpreset::CommonParams const& dataBlock)
{
    const std::unordered_map<std::string,
        electrification::ContactWireType> strToEnum
    {
        { "1w", electrification::ContactWireType::T1W },
        { "1w1", electrification::ContactWireType::T1W1 },
        { "1w2", electrification::ContactWireType::T1W2 },
        { "2w", electrification::ContactWireType::T2W },
        { "2w1", electrification::ContactWireType::T2W1 },
        { "2w2", electrification::ContactWireType::T2W2 }
    };

    overlapSpans = dataBlock.overlap_spans();

    contactWireTypeStr = dataBlock.contact_wire_type();
    auto findIt = strToEnum.find(contactWireTypeStr);
    assert(findIt != strToEnum.cend());
    if (findIt != strToEnum.cend())
    {
        contactWireType = findIt->second;
    }

    additionalContactWireTypeStr = contactWireTypeStr;
    additionalContactWireTypeStr =
        std::regex_replace(additionalContactWireTypeStr, std::regex("2w"), "1w");

    catenaryType = electrification::catenaryTypeFromAltProto(dataBlock.catenary_type());
    tensioningType = electrification::tensioningTypeFromAltProto(dataBlock.tensioning_type());
}

void ElectrificationAltMeshGeneratorConfig::CommonParamsData::saveToProto(
    altpreset::CommonParams& dataBlock)
{
    dataBlock.set_overlap_spans(overlapSpans);
    dataBlock.set_contact_wire_type(contactWireTypeStr);

    dataBlock.set_catenary_type(electrification::catenaryTypeToAltProto(catenaryType));
    dataBlock.set_tensioning_type(electrification::tensioningTypeToAltProto(tensioningType));
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct CatenaryData
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::CatenaryData::loadFromProto(
    altpreset::Catenary const& dataBlock)
{
    if (dataBlock.has_dropper_node())
    {
        dropperNode.loadFromProto(dataBlock.dropper_node());
    }

    if (dataBlock.has_insulator_dropper_node())
    {
        insulatorDropperNode.loadFromProto(dataBlock.insulator_dropper_node());
    }

    if (dataBlock.has_compensated_mid_point_anchor_node())
    {
        compMidPointAnchorNode.loadFromProto(dataBlock.compensated_mid_point_anchor_node());
    }

    if (dataBlock.has_semi_compensated_mid_point_anchor_node())
    {
        scompMidPointAnchorNode.loadFromProto(dataBlock.semi_compensated_mid_point_anchor_node());
    }

    if (dataBlock.has_tension_insulator())
    {
        tensionInsulator.loadFromProto(dataBlock.tension_insulator());
    }

    if (dataBlock.has_section_insulator())
    {
        sectionInsulator.loadFromProto(dataBlock.section_insulator());
    }

    if (dataBlock.has_overlap_insulator())
    {
        overlapInsulator.loadFromProto(dataBlock.overlap_insulator());
    }

    wireMaterial = dataBlock.wire_material();
    contactWireDiameter = dataBlock.contact_wire_diameter();
    catenaryWireDiameter = dataBlock.catenary_wire_diameter();
    dropperDiameter = dataBlock.dropper_diameter();
    saggingCoefficient = dataBlock.sagging_coefficient();
    maxVisibilityDistance = dataBlock.max_visibility_distance();
    maxFadeDistance = dataBlock.max_fade_distance();
    useMeshDropper = dataBlock.use_mesh_dropper();
    numOverlapInsulatorDroppers = dataBlock.num_overlap_insulator_droppers();
    overlapInsulatorDtopperStepM = dataBlock.overlap_insulator_droppers_step_m();
}

void ElectrificationAltMeshGeneratorConfig::CatenaryData::saveToProto(
    altpreset::Catenary& dataBlock)
{
    dropperNode.saveToProto(*dataBlock.mutable_dropper_node());
    insulatorDropperNode.saveToProto(*dataBlock.mutable_insulator_dropper_node());
    compMidPointAnchorNode.saveToProto(*dataBlock.mutable_compensated_mid_point_anchor_node());
    scompMidPointAnchorNode.saveToProto(*dataBlock.mutable_semi_compensated_mid_point_anchor_node());

    tensionInsulator.saveToProto(*dataBlock.mutable_tension_insulator());
    sectionInsulator.saveToProto(*dataBlock.mutable_section_insulator());
    overlapInsulator.saveToProto(*dataBlock.mutable_overlap_insulator());

    dataBlock.set_wire_material(wireMaterial);
    dataBlock.set_contact_wire_diameter(contactWireDiameter);
    dataBlock.set_catenary_wire_diameter(catenaryWireDiameter);
    dataBlock.set_dropper_diameter(dropperDiameter);
    dataBlock.set_sagging_coefficient(saggingCoefficient);
    dataBlock.set_max_visibility_distance(maxVisibilityDistance);
    dataBlock.set_max_fade_distance(maxFadeDistance);
    dataBlock.set_use_mesh_dropper(useMeshDropper);
    dataBlock.set_num_overlap_insulator_droppers(numOverlapInsulatorDroppers);
    dataBlock.set_overlap_insulator_droppers_step_m(overlapInsulatorDtopperStepM);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct PoleData::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::PoleData::loadFromProto(
    altpreset::Pole const& dataBlock)
{
    poleDoubleNode.loadFromProto(dataBlock.pole_double_node());
    poleNode = dataBlock.pole_node();
}

void ElectrificationAltMeshGeneratorConfig::PoleData::saveToProto(
    altpreset::Pole& dataBlock)
{
    poleDoubleNode.saveToProto(*dataBlock.mutable_pole_double_node());
    dataBlock.set_pole_node(poleNode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct PortalSuspensionData::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::PortalSuspensionData::loadFromProto(
    altpreset::PortalSuspension const& dataBlock)
{
    spnDoubleNode.loadFromProto(dataBlock.spn_double_node());
    spnNode = dataBlock.spn_node();
    spnZShiftM = dataBlock.spn_z_shift_m();
}

void ElectrificationAltMeshGeneratorConfig::PortalSuspensionData::saveToProto(
    altpreset::PortalSuspension& dataBlock)
{
    spnDoubleNode.saveToProto(*dataBlock.mutable_spn_double_node());
    dataBlock.set_spn_node(spnNode);
    dataBlock.set_spn_z_shift_m(spnZShiftM);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct PortalRigelData 
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::PortalData::loadFromProto(
    altpreset::Portal const& dataBlock)
{
    leftPoleNode = dataBlock.left_pole_node();
    rightPoleNode = dataBlock.right_pole_node();
    crossBarNode = dataBlock.crossbar_node();
    spanWireInsulatorNode = dataBlock.span_wire_insulator_node();
    crossWireInsulatorNode = dataBlock.cross_wire_insulator_node();
    dropperClamp = dataBlock.dropper_clamp();

    leftPortalPoleGap = dataBlock.left_portal_pole_gap();
    rightPortalPoleGap = dataBlock.right_portal_pole_gap();
    crossBarZShiftM = dataBlock.crossbar_z_shift_m();
    spanWireDiameter = dataBlock.span_wire_diameter();
    spanWireInsulatorTrackDistance = dataBlock.span_wire_insulator_track_distance();
    spanWireInsulatorDoubleDistance = dataBlock.span_wire_insulator_double_distance();
    spanWireInsulatorOffset = dataBlock.span_wire_insulator_offset();
    crossWireSaggingCoefficient = dataBlock.cross_wire_sagging_coefficient();
    spanWireConnectorOffset = dataBlock.span_wire_connector_offset();

    allowSpanWire = dataBlock.allow_span_wire();
    isCrossSpan = dataBlock.is_cross_span();

    if (dataBlock.has_span_wire_connector())
    {
        spanWireConnector.loadFromProto(dataBlock.span_wire_connector());
    }

}

void ElectrificationAltMeshGeneratorConfig::PortalData::saveToProto(
    altpreset::Portal& dataBlock)
{
    dataBlock.set_left_pole_node(leftPoleNode);
    dataBlock.set_right_pole_node(rightPoleNode);
    dataBlock.set_crossbar_node(crossBarNode);
    dataBlock.set_span_wire_insulator_node(spanWireInsulatorNode);
    dataBlock.set_cross_wire_insulator_node(crossWireInsulatorNode);
    dataBlock.set_dropper_clamp(dropperClamp);

    dataBlock.set_left_portal_pole_gap(leftPortalPoleGap);
    dataBlock.set_right_portal_pole_gap(rightPortalPoleGap);
    dataBlock.set_crossbar_z_shift_m(crossBarZShiftM);
    dataBlock.set_span_wire_diameter(spanWireDiameter);
    dataBlock.set_span_wire_insulator_track_distance(spanWireInsulatorTrackDistance);
    dataBlock.set_span_wire_insulator_double_distance(spanWireInsulatorDoubleDistance);
    dataBlock.set_span_wire_insulator_offset(spanWireInsulatorOffset);
    dataBlock.set_cross_wire_sagging_coefficient(crossWireSaggingCoefficient);
    dataBlock.set_span_wire_connector_offset(spanWireConnectorOffset);

    dataBlock.set_allow_span_wire(allowSpanWire);
    dataBlock.set_is_cross_span(isCrossSpan);

    spanWireConnector.saveToProto(*dataBlock.mutable_span_wire_connector());
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct AnchorData::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::AnchorData::loadFromProto(altpreset::Anchor const& dataBlock)
{
    fixedAnchorBlockNode = dataBlock.fixed_anchor_block_node();
    fixedBackTieNode = dataBlock.fixed_back_tie_node();
    compensatedAnchorBlockNode = dataBlock.compensated_anchor_block_node();
    compensatedBackTieNode = dataBlock.compensated_back_tie_node();
    semiCompensatedAnchorBlockNode = dataBlock.semi_compensated_anchor_block_node();
    semiCompensatedBackTieNode = dataBlock.semi_compensated_back_tie_node();
    midAnchorNode = dataBlock.mid_anchor_node();
    midBackTie = dataBlock.mid_back_tie();
}

void ElectrificationAltMeshGeneratorConfig::AnchorData::saveToProto(altpreset::Anchor& dataBlock)
{
    dataBlock.set_fixed_anchor_block_node(fixedAnchorBlockNode);
    dataBlock.set_fixed_back_tie_node(fixedBackTieNode);
    dataBlock.set_compensated_anchor_block_node(compensatedAnchorBlockNode);
    dataBlock.set_compensated_back_tie_node(compensatedBackTieNode);
    dataBlock.set_semi_compensated_anchor_block_node(semiCompensatedAnchorBlockNode);
    dataBlock.set_semi_compensated_back_tie_node(semiCompensatedBackTieNode);
    dataBlock.set_mid_anchor_node(midAnchorNode);
    dataBlock.set_mid_back_tie(midBackTie);
}

//=======================================================================================
// Nested data blocks
//=======================================================================================

/////////////////////////////////////////////////////////////////////////////////////////
// struct CatenaryAttachableNode::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::CatenaryAttachableNode::loadFromProto(
    altpreset::AttachableNode const& dataBlock)
{
    shortNode = dataBlock.short_node();
    mediumNode = dataBlock.medium_node();
    longNode = dataBlock.long_node();
}

void ElectrificationAltMeshGeneratorConfig::CatenaryAttachableNode::saveToProto(
    altpreset::AttachableNode& dataBlock)
{
    dataBlock.set_short_node(shortNode);
    dataBlock.set_medium_node(mediumNode);
    dataBlock.set_long_node(longNode);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct CatenaryWireInsulators::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::CatenaryWireInsulators::loadFromProto(
    altpreset::WireInsulators const& dataBlock)
{
    nodeUpper = dataBlock.node_upper();
    nodeLower = dataBlock.node_lower();
    distance = dataBlock.distance();
    useDroppers = dataBlock.use_droppers();
}

void ElectrificationAltMeshGeneratorConfig::CatenaryWireInsulators::saveToProto(
    altpreset::WireInsulators& dataBlock)
{
    dataBlock.set_node_upper(nodeUpper);
    dataBlock.set_node_lower(nodeLower);
    dataBlock.set_distance(distance);
    dataBlock.set_use_droppers(useDroppers);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct DoublePoleNodes::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::DoublePoleNodes::loadFromProto(
    altpreset::Pole_DoublePoleNodes const& dataBlock)
{
    poleDoubleNodePath = dataBlock.pole_double_node_path();
    poleA = dataBlock.pole_a();
    poleB = dataBlock.pole_b();
    poleC = dataBlock.pole_c();
    poleInsA = dataBlock.pole_ins_a();
    poleInsB = dataBlock.pole_ins_b();
    poleInsC = dataBlock.pole_ins_c();
    poleJunctionL = dataBlock.pole_junction_l();
    poleJunctionR = dataBlock.pole_junction_r();
}

void ElectrificationAltMeshGeneratorConfig::DoublePoleNodes::saveToProto(
    altpreset::Pole_DoublePoleNodes& dataBlock)
{
    dataBlock.set_pole_double_node_path(poleDoubleNodePath);
    dataBlock.set_pole_a(poleA);
    dataBlock.set_pole_b(poleB);
    dataBlock.set_pole_c(poleC);
    dataBlock.set_pole_ins_a(poleInsA);
    dataBlock.set_pole_ins_b(poleInsB);
    dataBlock.set_pole_ins_c(poleInsC);
    dataBlock.set_pole_junction_l(poleJunctionL);
    dataBlock.set_pole_junction_r(poleJunctionR);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct DoubleSuspensionNodes::
/////////////////////////////////////////////////////////////////////////////////////////
void ElectrificationAltMeshGeneratorConfig::DoubleSuspensionNodes::loadFromProto(
    altpreset::PortalSuspension_DoubleSuspensionNodes const& dataBlock)
{
    spnDoubleNodePath = dataBlock.spn_double_node_path();
    spnA = dataBlock.spn_a();
    spnB = dataBlock.spn_b();
    spnC = dataBlock.spn_c();
    spnInsA = dataBlock.spn_ins_a();
    spnInsB = dataBlock.spn_ins_b();
    spnInsC = dataBlock.spn_ins_c();
    spnJunctionL = dataBlock.spn_junction_l();
    spnJunctionR = dataBlock.spn_junction_r();
}

void ElectrificationAltMeshGeneratorConfig::DoubleSuspensionNodes::saveToProto(
    altpreset::PortalSuspension_DoubleSuspensionNodes& dataBlock)
{
    dataBlock.set_spn_double_node_path(spnDoubleNodePath);
    dataBlock.set_spn_a(spnA);
    dataBlock.set_spn_b(spnB);
    dataBlock.set_spn_c(spnC);
    dataBlock.set_spn_ins_a(spnInsA);
    dataBlock.set_spn_ins_b(spnInsB);
    dataBlock.set_spn_ins_c(spnInsC);
    dataBlock.set_spn_junction_l(spnJunctionL);
    dataBlock.set_spn_junction_r(spnJunctionR);
}

/////////////////////////////////////////////////////////////////////////////////////////
// struct CustomNodeData::
/////////////////////////////////////////////////////////////////////////////////////////
void
ElectrificationAltMeshGeneratorConfig::CustomNodeDataList::loadFromProto(
    proto::electrification::alternative_preset::ElectrificationAlternativePreset const& dataBlock)
{
    for (auto const& customPoleProto : dataBlock.custom_pole_nodes())
    {
        std::string nodeName = customPoleProto.node_name();
        std::string nodePath = customPoleProto.node_path();
        nodePathsByNames.emplace(nodeName, nodePath);
    }
}

void
ElectrificationAltMeshGeneratorConfig::CustomNodeDataList::saveToProto(
    proto::electrification::alternative_preset::ElectrificationAlternativePreset& dataBlock)
{
    for (auto& [nodeName, nodePath] : nodePathsByNames)
    {
        auto& customNodeProto = *dataBlock.add_custom_pole_nodes();
        customNodeProto.set_node_name(nodeName);
        customNodeProto.set_node_path(nodePath);
    }
}
