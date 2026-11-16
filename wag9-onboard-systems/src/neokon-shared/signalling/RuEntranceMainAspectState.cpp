#include "RuEntranceMainAspectState.h"
#include <cassert>

std::optional<RuEntranceMainAspectState> ruEntranceMainAspectStateFromInt(int state)
{
    switch (state)
    {
    case +RuEntranceMainAspectState::Green:
    case +RuEntranceMainAspectState::Yellow:
    case +RuEntranceMainAspectState::DoubleYellow:
    case +RuEntranceMainAspectState::FlashingYellow:
    case +RuEntranceMainAspectState::DoubleYellowTopFlashing:
    case +RuEntranceMainAspectState::DoubleYellowPlusGreenBar:
    case +RuEntranceMainAspectState::DoubleYellowPlusTwoGreenBars:
    case +RuEntranceMainAspectState::DoubleYellowTopFlashingPlusGreenBar:
    case +RuEntranceMainAspectState::DoubleYellowTopFlashingPlusTwoGreenBars:
    case +RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar:
    case +RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars:
    case +RuEntranceMainAspectState::Red:
        return static_cast<RuEntranceMainAspectState>(state);
    }

    return std::nullopt;
}

std::vector<RuEntranceMainAspectState>
ruEntranceMainAspectStatesWithGreenBars(int numGreenBars)
{
    if (numGreenBars == 2)
    {
        return 
        {
            RuEntranceMainAspectState::DoubleYellowPlusTwoGreenBars,
            RuEntranceMainAspectState::DoubleYellowTopFlashingPlusTwoGreenBars,
            RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusTwoGreenBars
        };
    }
    else if (numGreenBars == 1)
    {
        return 
        {
            RuEntranceMainAspectState::DoubleYellowPlusGreenBar,
            RuEntranceMainAspectState::DoubleYellowTopFlashingPlusGreenBar,
            RuEntranceMainAspectState::GreenFlashingPlusBottomYellowPlusGreenBar
        };
    }

    assert("Unsupported number of green bars");
    return { };
}

int toProto(RuEntranceMainAspectState state)
{
    return +state;
}

int toProto(std::optional<RuEntranceMainAspectState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<RuEntranceMainAspectState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<RuEntranceMainAspectState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ruEntranceMainAspectStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<RuEntranceMainAspectState>>& target)
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
