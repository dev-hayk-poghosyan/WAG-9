#pragma once

#include "ElectrificationPreset.pb.h"
#include "ElectrificationAltPreset.pb.h"
#include "neokon-shared/PlusEnum.h"

namespace electrification
{
enum class InsulatorType : int
{
    Tension = proto::electrification::preset::Insulator::Type::Insulator_Type_TENSION,
    Suspension = proto::electrification::preset::Insulator::Type::Insulator_Type_SUSPENSION,
    Cantilever = proto::electrification::preset::Insulator::Type::Insulator_Type_CANTILEVER,
    RegistrationArm = proto::electrification::preset::Insulator::Type::Insulator_Type_REGISTRATION_ARM,
    Section = proto::electrification::preset::Insulator::Type::Insulator_Type_SECTION,
    Spanwire = proto::electrification::preset::Insulator::Type::Insulator_Type_SPANWIRE
};

enum class InsulatorConstruct : int
{
    Plate = proto::electrification::preset::Insulator_Construct_Type_PLATE,
    Kernel = proto::electrification::preset::Insulator_Construct_Type_KERNEL
};

enum class InsulatorMaterial: int
{
    Porcelain = proto::electrification::preset::Insulator_Material_Type_PORCELAIN,
    Glass = proto::electrification::preset::Insulator_Material_Type_GLASS,
    Polymer = proto::electrification::preset::Insulator_Material_Type_POLYMER
};

enum class ProjectCode: int
{
    KS120 = proto::electrification::preset::CommonParams_ProjectCode_Type_KS120,
    KS160 = proto::electrification::preset::CommonParams_ProjectCode_Type_KS160,
    KS200 = proto::electrification::preset::CommonParams_ProjectCode_Type_KS200,
    KS250 = proto::electrification::preset::CommonParams_ProjectCode_Type_KS250
};

enum class ContactWireType: int
{
    T1W = proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W,
    T2W = proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W,
    T1W1 = proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W1,
    T1W2 = proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W2,
    T2W1 = proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W1,
    T2W2 = proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W2
};

enum class ConsoleProfileCut : int
{
    Tube = proto::electrification::preset::CommonParams_ConsoleProfileCut_Type_TUBE,
    Channel = proto::electrification::preset::CommonParams_ConsoleProfileCut_Type_CHANNEL
};

enum class PillarType: int
{
    T1X10 = proto::electrification::preset::Pillar_Type_T1X10,
    T1X12 = proto::electrification::preset::Pillar_Type_T1X12,
    T2X10 = proto::electrification::preset::Pillar_Type_T2X10,
    T2X12 = proto::electrification::preset::Pillar_Type_T2X12
};

enum class PillarMaterial: int
{
    Concrete = proto::electrification::preset::Pillar_Material_Type_CONCRETE,
    Metal = proto::electrification::preset::Pillar_Material_Type_METAL
};

enum class PortalBlockType: int
{
    RA = proto::electrification::preset::Portal_BlockType_Type_RA,
    RB = proto::electrification::preset::Portal_BlockType_Type_RB,
    WA = proto::electrification::preset::Portal_BlockType_Type_WA,
    WB = proto::electrification::preset::Portal_BlockType_Type_WB
};

enum class PortalBlockMaterial: int
{
    New = proto::electrification::preset::Portal_BlockMaterial_Type_NEW,
    Old = proto::electrification::preset::Portal_BlockMaterial_Type_OLD,
    Galvanized = proto::electrification::preset::Portal_BlockMaterial_Type_GALVANIZED
};

enum class SpanwireInsulatorPlacement: int
{
    Absent = proto::electrification::preset::Spanwire_InsulatorPlacement_Type_ABSENT,
    Left = proto::electrification::preset::Spanwire_InsulatorPlacement_Type_LEFT,
    Right = proto::electrification::preset::Spanwire_InsulatorPlacement_Type_RIGHT
};

inline InsulatorType insulatorTypeFromProto(
    proto::electrification::preset::Insulator::Type value)
{
    static_assert(+InsulatorType::Tension
        == +proto::electrification::preset::Insulator_Type_TENSION);
    static_assert(+InsulatorType::Suspension
        == +proto::electrification::preset::Insulator_Type_SUSPENSION);
    static_assert(+InsulatorType::Cantilever
        == +proto::electrification::preset::Insulator_Type_CANTILEVER);
    static_assert(+InsulatorType::RegistrationArm
        == +proto::electrification::preset::Insulator_Type_REGISTRATION_ARM);
    static_assert(+InsulatorType::Section
        == +proto::electrification::preset::Insulator_Type_SECTION);
    static_assert(+InsulatorType::Spanwire
        == +proto::electrification::preset::Insulator_Type_SPANWIRE);

    return static_cast<InsulatorType>(value);
};

inline proto::electrification::preset::Insulator::Type
insulatorTypeToProto(InsulatorType value)
{
    static_assert(+InsulatorType::Tension
        == +proto::electrification::preset::Insulator_Type_TENSION);
    static_assert(+InsulatorType::Suspension
        == +proto::electrification::preset::Insulator_Type_SUSPENSION);
    static_assert(+InsulatorType::Cantilever
        == +proto::electrification::preset::Insulator_Type_CANTILEVER);
    static_assert(+InsulatorType::RegistrationArm
        == +proto::electrification::preset::Insulator_Type_REGISTRATION_ARM);
    static_assert(+InsulatorType::Section
        == +proto::electrification::preset::Insulator_Type_SECTION);
    static_assert(+InsulatorType::Spanwire
        == +proto::electrification::preset::Insulator_Type_SPANWIRE);

    return static_cast<proto::electrification::preset::Insulator::Type>(value);
};

inline InsulatorConstruct insulatorConstructFromProto(
    proto::electrification::preset::Insulator::Construct::Type value)
{
    static_assert(+InsulatorConstruct::Plate ==
        +proto::electrification::preset::Insulator_Construct_Type_PLATE);
    static_assert(+InsulatorConstruct::Kernel ==
        +proto::electrification::preset::Insulator_Construct_Type_KERNEL);

    return static_cast<InsulatorConstruct>(value);
}

inline proto::electrification::preset::Insulator::Construct::Type
insulatorConstructToProto(InsulatorConstruct value)
{
    static_assert(+InsulatorConstruct::Plate ==
        +proto::electrification::preset::Insulator_Construct_Type_PLATE);
    static_assert(+InsulatorConstruct::Kernel ==
        +proto::electrification::preset::Insulator_Construct_Type_KERNEL);

    return static_cast<proto::electrification::preset::Insulator::Construct::Type>(value);
}

inline InsulatorMaterial insulatorMaterialFromProto(
    proto::electrification::preset::Insulator::Material::Type value)
{
    static_assert(+InsulatorMaterial::Porcelain ==
        +proto::electrification::preset::Insulator_Material_Type_PORCELAIN);
    static_assert(+InsulatorMaterial::Glass ==
        +proto::electrification::preset::Insulator_Material_Type_GLASS);
    static_assert(+InsulatorMaterial::Polymer ==
        +proto::electrification::preset::Insulator_Material_Type_POLYMER);

    return static_cast<InsulatorMaterial>(value);
};

inline proto::electrification::preset::Insulator::Material::Type
insulatorMaterialToProto(InsulatorMaterial value)
{
    static_assert(+InsulatorMaterial::Porcelain ==
        +proto::electrification::preset::Insulator_Material_Type_PORCELAIN);
    static_assert(+InsulatorMaterial::Glass ==
        +proto::electrification::preset::Insulator_Material_Type_GLASS);
    static_assert(+InsulatorMaterial::Polymer ==
        +proto::electrification::preset::Insulator_Material_Type_POLYMER);

    return static_cast<proto::electrification::preset::Insulator::Material::Type>(value);
};

inline ProjectCode projectCodeFromProto(
    proto::electrification::preset::CommonParams::ProjectCode::Type value)
{
    static_assert(+ProjectCode::KS120 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS120);
    static_assert(+ProjectCode::KS160 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS160);
    static_assert(+ProjectCode::KS200 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS200);
    static_assert(+ProjectCode::KS250 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS250);

    return static_cast<ProjectCode>(value);
};

inline proto::electrification::preset::CommonParams::ProjectCode::Type
projectCodeToProto(ProjectCode value)
{
    static_assert(+ProjectCode::KS120 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS120);
    static_assert(+ProjectCode::KS160 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS160);
    static_assert(+ProjectCode::KS200 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS200);
    static_assert(+ProjectCode::KS250 ==
        +proto::electrification::preset::CommonParams_ProjectCode_Type_KS250);

    return static_cast<proto::electrification::preset::CommonParams::ProjectCode::Type>(value);
};

inline ContactWireType catenaryWireTypeFromProto(
    proto::electrification::preset::CommonParams::CatenaryTypeIdx::Type value)
{
    static_assert(+ContactWireType::T1W ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W);
    static_assert(+ContactWireType::T2W ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W);
    static_assert(+ContactWireType::T1W1 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W1);
    static_assert(+ContactWireType::T1W2 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W2);
    static_assert(+ContactWireType::T2W1 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W1);
    static_assert(+ContactWireType::T2W2 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W2);

    return static_cast<ContactWireType>(value);
};

inline proto::electrification::preset::CommonParams::CatenaryTypeIdx::Type
catenaryWireTypeToProto(ContactWireType value)
{
    static_assert(+ContactWireType::T1W ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W);
    static_assert(+ContactWireType::T2W ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W);
    static_assert(+ContactWireType::T1W1 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W1);
    static_assert(+ContactWireType::T1W2 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T1W2);
    static_assert(+ContactWireType::T2W1 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W1);
    static_assert(+ContactWireType::T2W2 ==
        +proto::electrification::preset::CommonParams_CatenaryTypeIdx_Type_T2W2);

    return static_cast<proto::electrification::preset::CommonParams::CatenaryTypeIdx::Type>(value);
};

inline ConsoleProfileCut
consoleProfileCutFromProto(proto::electrification::preset::CommonParams::ConsoleProfileCut::Type value)
{
    static_assert(+ConsoleProfileCut::Tube ==
        +proto::electrification::preset::CommonParams_ConsoleProfileCut_Type_TUBE);
    static_assert(+ConsoleProfileCut::Channel ==
        +proto::electrification::preset::CommonParams_ConsoleProfileCut_Type_CHANNEL);

    return static_cast<ConsoleProfileCut>(value);
};

inline proto::electrification::preset::CommonParams::ConsoleProfileCut::Type
consoleProfileCutToProto(ConsoleProfileCut value)
{
    static_assert(+ConsoleProfileCut::Tube ==
        +proto::electrification::preset::CommonParams_ConsoleProfileCut_Type_TUBE);
    static_assert(+ConsoleProfileCut::Channel ==
        +proto::electrification::preset::CommonParams_ConsoleProfileCut_Type_CHANNEL);

    return static_cast<proto::electrification::preset
        ::CommonParams::ConsoleProfileCut::Type>(value);
};

inline PillarType pillarTypeFromProto(proto::electrification::preset::Pillar::Type value)
{
    static_assert(+PillarType::T1X10 == +proto::electrification::preset::Pillar_Type_T1X10);
    static_assert(+PillarType::T1X12 == +proto::electrification::preset::Pillar_Type_T1X12);
    static_assert(+PillarType::T2X10 == +proto::electrification::preset::Pillar_Type_T2X10);
    static_assert(+PillarType::T2X12 == +proto::electrification::preset::Pillar_Type_T2X12);

    return static_cast<PillarType>(value);
};

inline proto::electrification::preset::Pillar::Type
pillarTypeToProto(PillarType value)
{
    static_assert(+PillarType::T1X10 == +proto::electrification::preset::Pillar_Type_T1X10);
    static_assert(+PillarType::T1X12 == +proto::electrification::preset::Pillar_Type_T1X12);
    static_assert(+PillarType::T2X10 == +proto::electrification::preset::Pillar_Type_T2X10);
    static_assert(+PillarType::T2X12 == +proto::electrification::preset::Pillar_Type_T2X12);

    return static_cast<proto::electrification::preset::Pillar::Type>(value);
};

inline PillarMaterial pillarMaterialFromProto(
    proto::electrification::preset::Pillar::Material::Type value)
{
    static_assert(+PillarMaterial::Concrete ==
        +proto::electrification::preset::Pillar_Material_Type_CONCRETE);
    static_assert(+PillarMaterial::Metal ==
        +proto::electrification::preset::Pillar_Material_Type_METAL);

    return static_cast<PillarMaterial>(value);
};

inline proto::electrification::preset::Pillar::Material::Type pillarMaterialToProto(
    PillarMaterial value)
{
    static_assert(+PillarMaterial::Concrete ==
        +proto::electrification::preset::Pillar_Material_Type_CONCRETE);
    static_assert(+PillarMaterial::Metal ==
        +proto::electrification::preset::Pillar_Material_Type_METAL);

    return static_cast<proto::electrification::preset::Pillar::Material::Type>(value);
};

inline PortalBlockType portalBlockTypeFromProto(
    proto::electrification::preset::Portal::BlockType::Type value)
{
    static_assert(+PortalBlockType::RA ==
        +proto::electrification::preset::Portal_BlockType_Type_RA);
    static_assert(+PortalBlockType::RB ==
        +proto::electrification::preset::Portal_BlockType_Type_RB);
    static_assert(+PortalBlockType::WB ==
        +proto::electrification::preset::Portal_BlockType_Type_WB);
    static_assert(+PortalBlockType::WA ==
        +proto::electrification::preset::Portal_BlockType_Type_WA);

    return static_cast<PortalBlockType>(value);
};

inline proto::electrification::preset::Portal::BlockType::Type
portalBlockTypeToProto(PortalBlockType value)
{
    static_assert(+PortalBlockType::RA ==
        +proto::electrification::preset::Portal_BlockType_Type_RA);
    static_assert(+PortalBlockType::RB ==
        +proto::electrification::preset::Portal_BlockType_Type_RB);
    static_assert(+PortalBlockType::WB ==
        +proto::electrification::preset::Portal_BlockType_Type_WB);
    static_assert(+PortalBlockType::WA ==
        +proto::electrification::preset::Portal_BlockType_Type_WA);

    return static_cast<proto::electrification::preset::Portal::BlockType::Type>(value);
};

inline PortalBlockMaterial portalBlockMaterialFromProto(
    proto::electrification::preset::Portal::BlockMaterial::Type value)
{
    static_assert(+PortalBlockMaterial::New ==
        +proto::electrification::preset::Portal_BlockMaterial_Type_NEW);
    static_assert(+PortalBlockMaterial::Old ==
        +proto::electrification::preset::Portal_BlockMaterial_Type_OLD);
    static_assert(+PortalBlockMaterial::Galvanized ==
        +proto::electrification::preset::Portal_BlockMaterial_Type_GALVANIZED);

    return static_cast<PortalBlockMaterial>(value);
};

inline proto::electrification::preset::Portal::BlockMaterial::Type
portalBlockMaterialToProto(PortalBlockMaterial value)
{
    static_assert(+PortalBlockMaterial::New ==
        +proto::electrification::preset::Portal_BlockMaterial_Type_NEW);
    static_assert(+PortalBlockMaterial::Old ==
        +proto::electrification::preset::Portal_BlockMaterial_Type_OLD);
    static_assert(+PortalBlockMaterial::Galvanized ==
        +proto::electrification::preset::Portal_BlockMaterial_Type_GALVANIZED);

    return static_cast<proto::electrification::preset::Portal::BlockMaterial::Type>(value);
};

inline SpanwireInsulatorPlacement spanwireInsulatorPlacementFromProto(
    proto::electrification::preset::Spanwire::InsulatorPlacement::Type value)
{
    static_assert(+SpanwireInsulatorPlacement::Absent ==
        +proto::electrification::preset::Spanwire_InsulatorPlacement_Type_ABSENT);
    static_assert(+SpanwireInsulatorPlacement::Left ==
        +proto::electrification::preset::Spanwire_InsulatorPlacement_Type_LEFT);
    static_assert(+SpanwireInsulatorPlacement::Right ==
        +proto::electrification::preset::Spanwire_InsulatorPlacement_Type_RIGHT);

    return static_cast<SpanwireInsulatorPlacement>(value);
};

inline proto::electrification::preset::Spanwire::InsulatorPlacement::Type
spanwireInsulatorPlacementToProto(SpanwireInsulatorPlacement value)
{
    static_assert(+SpanwireInsulatorPlacement::Absent ==
        +proto::electrification::preset::Spanwire_InsulatorPlacement_Type_ABSENT);
    static_assert(+SpanwireInsulatorPlacement::Left ==
        +proto::electrification::preset::Spanwire_InsulatorPlacement_Type_LEFT);
    static_assert(+SpanwireInsulatorPlacement::Right ==
        +proto::electrification::preset::Spanwire_InsulatorPlacement_Type_RIGHT);

    return static_cast<proto::electrification::preset::Spanwire::InsulatorPlacement::Type>(value);
};

}
