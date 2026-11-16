#include "RuEntrance2MainAspectState.h"

std::optional<RuEntrance2MainAspectState> ruEntrance2MainAspectStateFromInt(int state)
{
    switch (state)
    {
    case +RuEntrance2MainAspectState::DoubleYellow:
    case +RuEntrance2MainAspectState::Red:
        return static_cast<RuEntrance2MainAspectState>(state);
    }

    return std::nullopt;
}

int toProto(RuEntrance2MainAspectState state)
{
    return +state;
}

int toProto(std::optional<RuEntrance2MainAspectState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<RuEntrance2MainAspectState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<RuEntrance2MainAspectState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ruEntrance2MainAspectStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<RuEntrance2MainAspectState>>& target)
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
