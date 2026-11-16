#include "CommonCabinSignalColor.h"

CommonCabinSignalColor commonCabinSignalColorFromLegacySignalLightState(
    LegacySignalLightState state)
{
    switch (state)
    {
    case LegacySignalLightState::Off:
        return CommonCabinSignalColor::White;
    case LegacySignalLightState::White:
    case LegacySignalLightState::WhiteFlash:
    case LegacySignalLightState::Err01:
        return CommonCabinSignalColor::White;
    case LegacySignalLightState::Green:
    case LegacySignalLightState::GreenWhite:
    case LegacySignalLightState::YellowFlash: // Not obvious but correct.
        return CommonCabinSignalColor::Green;
    case LegacySignalLightState::RedWhite: //Permits to move back from spec area.
    case LegacySignalLightState::Yellow:
    case LegacySignalLightState::YellowYellow:
    case LegacySignalLightState::YellowYellowFlash:
        return CommonCabinSignalColor::Yellow;
    case LegacySignalLightState::Red:
    case LegacySignalLightState::RedWhiteFlash:
        return CommonCabinSignalColor::RedYellow;
    }

    return CommonCabinSignalColor::Off;
}

std::optional<CommonCabinSignalColor> commonCabinSignalColorFromUuid(std::string const& uuid)
{
    if (uuid == "d533ba8a-036b-43e9-a784-cb02056eb6b2")
    {
        return CommonCabinSignalColor::Green;
    }
    else if (uuid == "a5bac933-dc37-43f6-993f-3393a988f60a")
    {
        return CommonCabinSignalColor::Yellow;
    }
    else if (uuid == "9a856055-cc77-4983-8982-2ae397fb689c")
    {
        return CommonCabinSignalColor::RedYellow;
    }
    else if (uuid == "e5c138bd-2814-4bba-8ecb-38085e970f86")
    {
        return CommonCabinSignalColor::Red;
    }
    else if (uuid == "c2e2e649-2fe9-48d7-a559-bea2fe9c7ac8")
    {
        return CommonCabinSignalColor::White;
    }
    else if (uuid == "54f4cc30-6522-43f9-98ab-f126603e740e")
    {
        return CommonCabinSignalColor::Off;
    }

    return std::nullopt;
}
