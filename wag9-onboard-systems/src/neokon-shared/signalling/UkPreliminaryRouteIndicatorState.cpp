#include "UkPreliminaryRouteIndicatorState.h"


std::optional<UkPreliminaryRouteIndicatorState> ukPreliminaryRouteIndicatorStateFromInt(int state)
{
    switch (state)
    {
    case +UkPreliminaryRouteIndicatorState::NoInidicator:
    case +UkPreliminaryRouteIndicatorState::Top:
    case +UkPreliminaryRouteIndicatorState::TopLeft:
    case +UkPreliminaryRouteIndicatorState::Left:
    case +UkPreliminaryRouteIndicatorState::BottomLeft:
    case +UkPreliminaryRouteIndicatorState::TopRight:
    case +UkPreliminaryRouteIndicatorState::Right:
    case +UkPreliminaryRouteIndicatorState::BottomRight:
        return static_cast<UkPreliminaryRouteIndicatorState>(state);
    }

    return std::nullopt;
}

UkPreliminaryRouteIndicatorState ukPreliminaryRouteIndicatorStateFromJunctionState(
    std::optional<UkJunctionIndicatorArm> arm, bool nonRedSignal)
{
    if (!nonRedSignal)
    {
        return UkPreliminaryRouteIndicatorState::NoInidicator;
    }

    if (!arm)
    {
        // Since it's non-red and non-off signal, and there is no arm, the arrow should point up.
        return UkPreliminaryRouteIndicatorState::Top;
    }

    switch (*arm)
    {
    case UkJunctionIndicatorArm::TopLeft:
        return UkPreliminaryRouteIndicatorState::TopLeft;
    case UkJunctionIndicatorArm::Left:
        return UkPreliminaryRouteIndicatorState::Left;
    case UkJunctionIndicatorArm::BottomLeft:
        return UkPreliminaryRouteIndicatorState::BottomLeft;
    case UkJunctionIndicatorArm::TopRight:
        return UkPreliminaryRouteIndicatorState::TopRight;
    case UkJunctionIndicatorArm::Right:
        return UkPreliminaryRouteIndicatorState::Right;
    case UkJunctionIndicatorArm::BottomRight:
        return UkPreliminaryRouteIndicatorState::BottomRight;
    }

    return UkPreliminaryRouteIndicatorState::NoInidicator;
}

int toProto(UkPreliminaryRouteIndicatorState state)
{
    return +state;
}

int toProto(std::optional<UkPreliminaryRouteIndicatorState> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

int toProto(std::optional<std::optional<UkPreliminaryRouteIndicatorState>> const& state)
{
    return state ? 1 + toProto(*state) : 0;
}

void fromProto(int protoVal, std::optional<UkPreliminaryRouteIndicatorState>& target)
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = ukPreliminaryRouteIndicatorStateFromInt(protoVal - 1);
    }
}

void fromProto(int protoVal, std::optional<std::optional<UkPreliminaryRouteIndicatorState>>& target)
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
