#include "UkJunctionIndicatorArm.h"

namespace
{
struct Mapping
{
    wchar_t const* label;
    char const* nameSuffix;
    UkJunctionIndicatorArm arm;
};

static Mapping const ArmMappings[] = {
    { L"TL", "TL", UkJunctionIndicatorArm::TopLeft },
    { L"L",  "L",  UkJunctionIndicatorArm::Left },
    { L"BL", "BL", UkJunctionIndicatorArm::BottomLeft },
    { L"TR", "TR", UkJunctionIndicatorArm::TopRight },
    { L"R",  "R",  UkJunctionIndicatorArm::Right },
    { L"BR", "BR", UkJunctionIndicatorArm::BottomRight },
};

static Mapping const ArmMappingsLed[] = {
    { L"1", "1", UkJunctionIndicatorArm::TopLeft },
    { L"2", "2", UkJunctionIndicatorArm::Left },
    { L"3", "3", UkJunctionIndicatorArm::BottomLeft },
    { L"4", "4", UkJunctionIndicatorArm::TopRight },
    { L"5", "5", UkJunctionIndicatorArm::Right },
    { L"6", "6", UkJunctionIndicatorArm::BottomRight },
};

} // anonymous namespace

std::optional<UkJunctionIndicatorArm> ukJunctionIndicatorArmFromInt(int const state)
{
    switch (state)
    {
    case +UkJunctionIndicatorArm::TopLeft:
    case +UkJunctionIndicatorArm::Left:
    case +UkJunctionIndicatorArm::BottomLeft:
    case +UkJunctionIndicatorArm::TopRight:
    case +UkJunctionIndicatorArm::Right:
    case +UkJunctionIndicatorArm::BottomRight:
        return static_cast<UkJunctionIndicatorArm>(state);
    }

    return std::nullopt;
}

std::optional<UkJunctionIndicatorArm> ukJunctionIndicatorArmFromLabel(std::wstring const& label)
{
    for (Mapping const& mapping : ArmMappings)
    {
        if (label == mapping.label)
        {
            return mapping.arm;
        }
    }

    return std::nullopt;
}

std::optional<const char*> ukJunctionIndicatorArmToNameSuffix(int state)
{
    auto arm = ukJunctionIndicatorArmFromInt(state);
    if (!arm)
    {
        return {};
    }

    for (Mapping const& mapping : ArmMappings)
    {
        if (mapping.arm == *arm)
        {
            return mapping.nameSuffix;
        }
    }

    for (Mapping const& mapping : ArmMappingsLed)
    {
        if (mapping.arm == *arm)
        {
            return mapping.nameSuffix;
        }
    }

    return {};
}

int toProto(UkJunctionIndicatorArm state)
{
    return +state;
}

int toProto(std::optional<UkJunctionIndicatorArm> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<UkJunctionIndicatorArm>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<UkJunctionIndicatorArm>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ukJunctionIndicatorArmFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<UkJunctionIndicatorArm>>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target.emplace();
        fromProto(protoVal - 1, *target);
    }
}
