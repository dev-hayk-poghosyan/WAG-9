#include "UkRightAwayState.h"

std::optional<UkRightAwayState> ukRightAwayStateFromInt(int const state)
{
    switch (state)
    {
    case +UkRightAwayState::Off:
    case +UkRightAwayState::On:
    case +UkRightAwayState::CD:
        return static_cast<UkRightAwayState>(state);
    }

    return std::nullopt;
}

int toProto(UkRightAwayState state)
{
    return +state;
}

int toProto(std::optional<UkRightAwayState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<UkRightAwayState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<UkRightAwayState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ukRightAwayStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<UkRightAwayState>>& target)
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
