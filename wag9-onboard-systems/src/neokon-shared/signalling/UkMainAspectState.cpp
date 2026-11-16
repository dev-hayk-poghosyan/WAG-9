#include "UkMainAspectState.h"

std::optional<UkMainAspectState> ukMainAspectStateFromInt(int const state)
{
    switch (state)
    {
    case +UkMainAspectState::Green:
    case +UkMainAspectState::Yellow:
    case +UkMainAspectState::Red:
    case +UkMainAspectState::DoubleYellow:
    case +UkMainAspectState::FlashingYellow:
    case +UkMainAspectState::FlashingDoubleYellow:
    case +UkMainAspectState::Off:
        return static_cast<UkMainAspectState>(state);
    }

    return std::nullopt;
}

bool validForUk2AspectSignal(UkMainAspectState state)
{
    switch (state)
    {
    case UkMainAspectState::Green:
    case UkMainAspectState::Red:
    case UkMainAspectState::Off:
        return true;
    default:
        return false;
    }
}

bool validForUk3AspectSignal(UkMainAspectState state)
{
    switch (state)
    {
    case UkMainAspectState::Green:
    case UkMainAspectState::Yellow:
    case UkMainAspectState::Red:
    case UkMainAspectState::FlashingYellow:
    case UkMainAspectState::Off:
        return true;
    default:
        return false;
    }
}

bool validForUk4AspectSignal(UkMainAspectState state)
{
    switch (state)
    {
    case UkMainAspectState::Green:
    case UkMainAspectState::Yellow:
    case UkMainAspectState::Red:
    case UkMainAspectState::DoubleYellow:
    case UkMainAspectState::FlashingYellow:
    case UkMainAspectState::FlashingDoubleYellow:
    case UkMainAspectState::Off:
        return true;
    default:
        return false;
    }
}

int toProto(UkMainAspectState state)
{
    return +state;
}

int toProto(std::optional<UkMainAspectState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<UkMainAspectState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int symbolicIndicatorToProto(std::optional<int> const& state)
{
    return state ? (1 + *state) : 0;
}

int manualSymbolicIndicatorToProto(std::optional<std::optional<int>> const& state)
{
    return state ? 1 + symbolicIndicatorToProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<UkMainAspectState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ukMainAspectStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<UkMainAspectState>>& target)
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

void symbolicIndicatorFromProto(int protoVal, std::optional<int>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = protoVal - 1;
    }
}

void manualSymbolicIndicatorFromProto(int protoVal, std::optional<std::optional<int>>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target.emplace();
        symbolicIndicatorFromProto(protoVal - 1, *target);
    }
}
