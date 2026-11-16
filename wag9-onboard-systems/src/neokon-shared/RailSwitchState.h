#pragma once

#include "SwitchState.pb.h"

/**
 * A list of all possible states of all supported switches.
 */
enum class RailSwitchState
{
    YswLeft                  = proto::YSW_STATE_LEFT,
    YswRight                 = proto::YSW_STATE_RIGHT,

    XswLeftInputLeftOutput   = proto::XSW_STATE_LEFT_INPUT_LEFT_OUTPUT,
    XswLeftInputRightOutput  = proto::XSW_STATE_LEFT_INPUT_RIGHT_OUTPUT,
    XswRightInputLeftOutput  = proto::XSW_STATE_RIGHT_INPUT_LEFT_OUTPUT,
    XswRightInputRightOutput = proto::XSW_STATE_RIGHT_INPUT_RIGHT_OUTPUT,

    TwswNearSide             = proto::TWSW_STATE_NEAR_SIDE,
    TwswFarLeft              = proto::TWSW_STATE_FAR_LEFT,
    TwswFarRight             = proto::TWSW_STATE_FAR_RIGHT,

    DswStraightPassing       = proto::DSW_STATE_STRAIGHT_PASSING,
    DswLeftInputRightOutput  = proto::DSW_STATE_LEFT_INPUT_RIGHT_OUTPUT,
    DswRightInputLeftOutput  = proto::DSW_STATE_RIGHT_INPUT_LEFT_OUTPUT,
};

inline RailSwitchState railSwitchStateFromProto(proto::SwitchState switchState)
{
    return static_cast<RailSwitchState>(switchState);
}

inline proto::SwitchState railSwitchStateToProto(RailSwitchState railSwitchState)
{
    return static_cast<proto::SwitchState>(railSwitchState);
}
