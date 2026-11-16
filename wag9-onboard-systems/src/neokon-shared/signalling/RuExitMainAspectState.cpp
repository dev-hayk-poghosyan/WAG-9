#include "RuExitMainAspectState.h"
#include "RuSymbolDisplayState.h"

std::optional<RuExitMainAspectState> ruExitMainAspectStateFromInt(int state)
{
    switch (state)
    {
    case +RuExitMainAspectState::Green:
    case +RuExitMainAspectState::Yellow:
    case +RuExitMainAspectState::DoubleYellow:
    case +RuExitMainAspectState::DoubleYellowPlusGreenBar:
    case +RuExitMainAspectState::DoubleYellowPlusTwoGreenBars:
    case +RuExitMainAspectState::DoubleYellowTopFlashing:
    case +RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar:
    case +RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars:
    case +RuExitMainAspectState::Red:
        return static_cast<RuExitMainAspectState>(state);
    }

    return std::nullopt;
}

RuExitMainAspectState ruExitMainAspectStateDoubleYellowOrReplacement(int numGreenBars)
{
    if (numGreenBars == 2)
    {
        return RuExitMainAspectState::DoubleYellowPlusTwoGreenBars;
    }
    else if (numGreenBars == 1)
    {
        return RuExitMainAspectState::DoubleYellowPlusGreenBar;
    }
    else
    {
        return RuExitMainAspectState::DoubleYellow;
    }
}

RuExitMainAspectState ruExitMainAspectStateDoubleYellowTopFlashingOrReplacement(int numGreenBars)
{
    if (numGreenBars == 2)
    {
        return RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars;
    }
    else if (numGreenBars == 1)
    {
        return RuExitMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar;
    }
    else
    {
        return RuExitMainAspectState::DoubleYellowTopFlashing;
    }
}

int toProto(RuExitMainAspectState state)
{
    return +state;
}

int toProto(std::optional<RuExitMainAspectState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<RuExitMainAspectState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<RuExitMainAspectState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ruExitMainAspectStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<RuExitMainAspectState>>& target)
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
