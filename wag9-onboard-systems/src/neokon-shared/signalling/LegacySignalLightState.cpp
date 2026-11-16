#include "LegacySignalLightState.h"

#include "neokon-shared/PlusEnum.h"

std::optional<LegacySignalLightState> legacySignalLightStateFromInt(int state)
{
    switch (state)
    {
    case +LegacySignalLightState::Off:
    case +LegacySignalLightState::Blue:
    case +LegacySignalLightState::Err01:
    case +LegacySignalLightState::Green:
    case +LegacySignalLightState::GreenWhite:
    case +LegacySignalLightState::Red:
    case +LegacySignalLightState::RedWhite:
    case +LegacySignalLightState::RedWhiteFlash:
    case +LegacySignalLightState::White:
    case +LegacySignalLightState::WhiteFlash:
    case +LegacySignalLightState::WhiteWhite:
    case +LegacySignalLightState::Yellow:
    case +LegacySignalLightState::YellowFlash:
    case +LegacySignalLightState::YellowYellow:
    case +LegacySignalLightState::YellowYellowFlash:
        return static_cast<LegacySignalLightState>(state);
    }

    return std::nullopt;
}

int toProto(LegacySignalLightState state)
{
    return +state;
}

int toProto(std::optional<LegacySignalLightState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<LegacySignalLightState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<LegacySignalLightState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = legacySignalLightStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<LegacySignalLightState>>& target)
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
