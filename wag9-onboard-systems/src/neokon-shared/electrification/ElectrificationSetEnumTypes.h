#pragma once

#include "ElectrificationEnumTypes.pb.h"
#include "ElectrificationAltPresetCatenary.pb.h"
#include "ElectrificationAltPresetCommonParams.pb.h"
#include "ElectrificationConsole.pb.h"
#include "ElectrificationTensionOverlap.pb.h"
#include "ElectrificationPortalPillar.pb.h"
#include "neokon-shared/PlusEnum.h"

namespace electrification
{

enum class ConsoleType : int
{
    Isolated = proto::electrification::Console_Type_ISOLATED,
    Cantilever = proto::electrification::Console_Type_CANTILEVER,
    Horizontal = proto::electrification::Console_Type_HORIZONTAL,
    Bent = proto::electrification::Console_Type_BENT
};

enum class CatenaryType : int
{
    Simple = proto::electrification::Catenary_Type_SIMPLE,
    Stitched = proto::electrification::Catenary_Type_STITCHED
};

enum class TensioningType : int
{
    None = proto::electrification::Tensioning_Type_NONE,
    Fixed = proto::electrification::Tensioning_Type_FIXED,
    Compensated = proto::electrification::Tensioning_Type_COMPENSATED,
    SemiCompensated = proto::electrification::Tensioning_Type_SEMICOMPENSATED
};

enum class PoleSide : int
{
    LeftHand = proto::electrification::PoleSide_Type_LEFTHAND,
    RightHand = proto::electrification::PoleSide_Type_RIGHTHAND
};

enum class TensionOverlapType : int
{
    Insulated = proto::electrification::TensionOverlap_Type_INSULATED,
    NonInsulated = proto::electrification::TensionOverlap_Type_NONINSULATED
};

/**
 * Положение объекта на родительском объекте TensionPath.
 * Необходимо для задания особых условий для начала и конца электрифицированного участко.
 * Одиночные объекты и объекты в середине пути имеют тип None.
 */
enum class TensionPathPlacement : int
{
    None = proto::electrification::PathPlacement_Type_NONE,
    Inner = proto::electrification::PathPlacement_Type_INNER,
    Start = proto::electrification::PathPlacement_Type_START,
    End = proto::electrification::PathPlacement_Type_END
};

enum class PortalPillarSide : int
{
    Left = proto::electrification::PortalPillar_Side_LEFT,
    Right = proto::electrification::PortalPillar_Side_RIGHT
};

inline PortalPillarSide
portalPillarSideFromProto(proto::electrification::PortalPillar_Side const& value)
{
    static_assert(+PortalPillarSide::Left == +proto::electrification::PortalPillar_Side_LEFT);
    static_assert(+PortalPillarSide::Right == +proto::electrification::PortalPillar_Side_RIGHT);

    return static_cast<PortalPillarSide>(value);
}

inline proto::electrification::PortalPillar_Side
portalPillarSideToProto(PortalPillarSide const& value)
{
    static_assert(+PortalPillarSide::Left == +proto::electrification::PortalPillar_Side_LEFT);
    static_assert(+PortalPillarSide::Right == +proto::electrification::PortalPillar_Side_RIGHT);

    return static_cast<proto::electrification::PortalPillar_Side>(value);
}

inline ConsoleType  consoleTypeFromProto(
    proto::electrification::Console::Type const& value)
{
    static_assert(+ConsoleType::Isolated == +proto::electrification::Console_Type_ISOLATED);
    static_assert(+ConsoleType::Horizontal == +proto::electrification::Console_Type_HORIZONTAL);
    static_assert(+ConsoleType::Cantilever == +proto::electrification::Console_Type_CANTILEVER);
    static_assert(+ConsoleType::Bent == +proto::electrification::Console_Type_BENT);

    return static_cast<ConsoleType>(value);
}

inline proto::electrification::Console::Type
consoleTypeToProto(ConsoleType const& value)
{
    static_assert(+ConsoleType::Isolated == +proto::electrification::Console_Type_ISOLATED);
    static_assert(+ConsoleType::Horizontal == +proto::electrification::Console_Type_HORIZONTAL);
    static_assert(+ConsoleType::Cantilever == +proto::electrification::Console_Type_CANTILEVER);
    static_assert(+ConsoleType::Bent == +proto::electrification::Console_Type_BENT);

    return static_cast<proto::electrification::Console::Type>(value);
}

inline CatenaryType catenaryTypeFromProto(
    proto::electrification::Catenary::Type const& value)
{
    static_assert(+CatenaryType::Simple == +proto::electrification::Catenary_Type_SIMPLE);
    static_assert(+CatenaryType::Stitched == +proto::electrification::Catenary_Type_STITCHED);

    return static_cast<CatenaryType>(value);
}


inline proto::electrification::Catenary::Type
catenaryTypeToProto(CatenaryType const& value)
{
    static_assert(+CatenaryType::Simple == +proto::electrification::Catenary_Type_SIMPLE);
    static_assert(+CatenaryType::Stitched == +proto::electrification::Catenary_Type_STITCHED);

    return static_cast<proto::electrification::Catenary::Type>(value);
}

inline TensioningType tensioningTypeFromProto(
    proto::electrification::Tensioning::Type const& value)
{
    static_assert(+TensioningType::None == +proto::electrification::Tensioning_Type_NONE);
    static_assert(+TensioningType::Fixed == +proto::electrification::Tensioning_Type_FIXED);
    static_assert(+TensioningType::Compensated == 
        +proto::electrification::Tensioning_Type_COMPENSATED);
    static_assert(+TensioningType::SemiCompensated == 
        +proto::electrification::Tensioning_Type_SEMICOMPENSATED);

    return static_cast<TensioningType>(value);
}

inline proto::electrification::Tensioning::Type 
tensioningTypeToProto(TensioningType const& value)
{
    static_assert(+TensioningType::None == +proto::electrification::Tensioning_Type_NONE);
    static_assert(+TensioningType::Fixed == +proto::electrification::Tensioning_Type_FIXED);
    static_assert(+TensioningType::Compensated == 
        +proto::electrification::Tensioning_Type_COMPENSATED);
    static_assert(+TensioningType::SemiCompensated == 
        +proto::electrification::Tensioning_Type_SEMICOMPENSATED);

    return static_cast<proto::electrification::Tensioning::Type>(value);
}

inline TensionPathPlacement
pathPlacementFromProto(proto::electrification::PathPlacement::Type const& value)
{
    static_assert(+TensionPathPlacement::None == +proto::electrification::PathPlacement_Type_NONE);
    static_assert(+TensionPathPlacement::Inner == +proto::electrification::PathPlacement_Type_INNER);
    static_assert(+TensionPathPlacement::Start == +proto::electrification::PathPlacement_Type_START);
    static_assert(+TensionPathPlacement::End == +proto::electrification::PathPlacement_Type_END);

    return static_cast<TensionPathPlacement>(value);
}

inline proto::electrification::PathPlacement::Type
pathPlacementToProto(TensionPathPlacement const& value)
{
    static_assert(+TensionPathPlacement::None == +proto::electrification::PathPlacement_Type_NONE);
    static_assert(+TensionPathPlacement::Inner == +proto::electrification::PathPlacement_Type_INNER);
    static_assert(+TensionPathPlacement::Start == +proto::electrification::PathPlacement_Type_START);
    static_assert(+TensionPathPlacement::End == +proto::electrification::PathPlacement_Type_END);

    return static_cast<proto::electrification::PathPlacement::Type>(value);
}

inline PoleSide poleSideFromProto(
    proto::electrification::PoleSide::Type const& value)
{
    static_assert(+PoleSide::LeftHand == +proto::electrification::PoleSide_Type_LEFTHAND);
    static_assert(+PoleSide::RightHand == +proto::electrification::PoleSide_Type_RIGHTHAND);

    return static_cast<PoleSide>(value);
}

inline proto::electrification::PoleSide::Type
poleSideToProto(PoleSide const& value)
{
    static_assert(+PoleSide::LeftHand == +proto::electrification::PoleSide_Type_LEFTHAND);
    static_assert(+PoleSide::RightHand == +proto::electrification::PoleSide_Type_RIGHTHAND);

    return static_cast<proto::electrification::PoleSide::Type>(value);
}

inline TensionOverlapType tensionOverlapFromProto(
    proto::electrification::TensionOverlap_Type const& value)
{
    static_assert(+TensionOverlapType::Insulated == +proto::electrification::TensionOverlap_Type_INSULATED);
    static_assert(+TensionOverlapType::NonInsulated == +proto::electrification::TensionOverlap_Type_NONINSULATED);

    return static_cast<TensionOverlapType>(value);
}

inline proto::electrification::TensionOverlap_Type
tensionOverlapToProto(TensionOverlapType const& value)
{
    static_assert(+TensionOverlapType::Insulated == +proto::electrification::TensionOverlap_Type_INSULATED);
    static_assert(+TensionOverlapType::NonInsulated == +proto::electrification::TensionOverlap_Type_NONINSULATED);

    return static_cast<proto::electrification::TensionOverlap_Type>(value);
}

inline CatenaryType catenaryTypeFromAltProto(
    proto::electrification::alternative_preset::Catenary::Type const& value)
{
    static_assert(+CatenaryType::Simple == +proto::electrification::alternative_preset::Catenary_Type_SIMPLE);
    static_assert(+CatenaryType::Stitched == +proto::electrification::alternative_preset::Catenary_Type_STITCHED);

    return static_cast<CatenaryType>(value);
}


inline proto::electrification::alternative_preset::Catenary::Type
catenaryTypeToAltProto(CatenaryType const& value)
{
    static_assert(+CatenaryType::Simple == +proto::electrification::alternative_preset::Catenary_Type_SIMPLE);
    static_assert(+CatenaryType::Stitched == +proto::electrification::alternative_preset::Catenary_Type_STITCHED);

    return static_cast<proto::electrification::alternative_preset::Catenary::Type>(value);
}

inline TensioningType
tensioningTypeFromAltProto(
    proto::electrification::alternative_preset::Tensioning::Type const& value)
{
    static_assert(+TensioningType::None ==
        +proto::electrification::alternative_preset::Tensioning_Type_NONE);
    static_assert(+TensioningType::Fixed ==
        +proto::electrification::alternative_preset::Tensioning_Type_FIXED);
    static_assert(+TensioningType::Compensated ==
        +proto::electrification::alternative_preset::Tensioning_Type_COMPENSATED);
    static_assert(+TensioningType::SemiCompensated ==
        +proto::electrification::alternative_preset::Tensioning_Type_SEMICOMPENSATED);

    return static_cast<TensioningType>(value);
}

inline proto::electrification::alternative_preset::Tensioning::Type
tensioningTypeToAltProto(TensioningType const& value)
{
    static_assert(+TensioningType::None == +proto::electrification::Tensioning_Type_NONE);
    static_assert(+TensioningType::Fixed == +proto::electrification::Tensioning_Type_FIXED);
    static_assert(
        +TensioningType::Compensated == +proto::electrification::Tensioning_Type_COMPENSATED);
    static_assert(+TensioningType::SemiCompensated ==
        +proto::electrification::Tensioning_Type_SEMICOMPENSATED);

    return static_cast<proto::electrification::alternative_preset::Tensioning::Type>(value);
}

inline PoleSide operator~(PoleSide const& value)
{
    return value == PoleSide::LeftHand ? PoleSide::RightHand : PoleSide::LeftHand;
}

}//electrification
