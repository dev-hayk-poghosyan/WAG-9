#pragma once

#include "SignalLightStateTrigger.pb.h"

#include "neokon-shared/PlusEnum.h"

enum class SignalLightStateTriggerReact
{
    All = proto::SignalLightStateTrigger::REACT_ALL,
    BotsOnly = proto::SignalLightStateTrigger::REACT_BOTS_ONLY,
    StudentOnly = proto::SignalLightStateTrigger::REACT_STUDENT_ONLY,
};

static_assert(+SignalLightStateTriggerReact::All == proto::SignalLightStateTrigger::REACT_ALL);
static_assert(+SignalLightStateTriggerReact::BotsOnly ==
    proto::SignalLightStateTrigger::REACT_BOTS_ONLY);
static_assert(+SignalLightStateTriggerReact::StudentOnly ==
    proto::SignalLightStateTrigger::REACT_STUDENT_ONLY);

inline proto::SignalLightStateTrigger::React reactToProto(SignalLightStateTriggerReact react)
{
    return static_cast<proto::SignalLightStateTrigger::React>(react);
}

inline SignalLightStateTriggerReact reactfromProto(proto::SignalLightStateTrigger::React react)
{
    return static_cast<SignalLightStateTriggerReact>(react);
}
