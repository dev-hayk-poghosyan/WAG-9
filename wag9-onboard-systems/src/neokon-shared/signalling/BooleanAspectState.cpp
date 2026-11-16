#include "BooleanAspectState.h"

std::optional<BooleanAspectState> booleanAspectStateFromInt(int state)
{
    switch (state)
    {
    case +BooleanAspectState::Off:
    case +BooleanAspectState::On:
        return static_cast<BooleanAspectState>(state);
    }

    return std::nullopt;
}

int toProto(BooleanAspectState state)
{
    return +state;
}

int toProto(std::optional<BooleanAspectState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<BooleanAspectState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<BooleanAspectState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = booleanAspectStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<BooleanAspectState>>& target)
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
