#pragma once

#include "TogglerState.pb.h"

/**
 * State of binary toggler.
 */
enum class TogglerState
{
    Off = proto::TogglerState::OFF,
    On = proto::TogglerState::ON
};

inline proto::TogglerState::State togglerStateToProto(TogglerState state)
{
    return static_cast<proto::TogglerState::State>(state);
}

inline TogglerState togglerStateFromProto(proto::TogglerState::State state)
{
    return static_cast<TogglerState>(state);
}

inline proto::TogglerState::State boolToProtoTogglerState(bool switchState)
{
    return switchState ? proto::TogglerState::ON : proto::TogglerState::OFF;
}

inline bool protoTogglerStateToBool(proto::TogglerState::State state)
{
    return state == proto::TogglerState::ON;
}

inline bool togglerStateToBool(TogglerState state)
{
    return state == TogglerState::On;
}

inline TogglerState boolToTogglerState(bool switchState)
{
    return switchState ? TogglerState::On : TogglerState::Off;
}
