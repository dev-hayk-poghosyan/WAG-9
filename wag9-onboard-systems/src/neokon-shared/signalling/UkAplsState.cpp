#include "UkAplsState.h"

std::optional<UkAplsState> ukAplsStateFromInt(int const state)
{
    switch (state)
    {
    case +UkAplsState::ObeyMainSignal:
    case +UkAplsState::ProceedCautiously:
        return static_cast<UkAplsState>(state);
    }

    return std::nullopt;
}

int toProto(UkAplsState state)
{
    return +state;
}

int toProto(std::optional<UkAplsState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<UkAplsState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<UkAplsState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ukAplsStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<UkAplsState>>& target)
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

