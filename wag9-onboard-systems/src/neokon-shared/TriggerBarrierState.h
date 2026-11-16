#pragma once

#include "TriggerBarrierState.pb.h"

#include "neokon-shared/PlusEnum.h"

/**
 * States of barrier that may be associated with animation trigger.
 */
enum class TriggerBarrierState : int
{
    Closed = proto::TriggerBarrierState::CLOSED,
    Opened = proto::TriggerBarrierState::OPENED
};

inline proto::TriggerBarrierState::State protoFromTriggerBarrierState(TriggerBarrierState state)
{
    static_assert(+TriggerBarrierState::Closed == proto::TriggerBarrierState::CLOSED);
    static_assert(+TriggerBarrierState::Opened == proto::TriggerBarrierState::OPENED);

    return static_cast<proto::TriggerBarrierState::State>(state);
}

inline TriggerBarrierState oppositeBarrierState(TriggerBarrierState state)
{
    return state == TriggerBarrierState::Closed ? TriggerBarrierState::Opened :
        TriggerBarrierState::Closed;
}

inline TriggerBarrierState
triggerBarrierStateFromProto(proto::TriggerBarrierState::State barrierState)
{
    static_assert(+TriggerBarrierState::Closed == proto::TriggerBarrierState::CLOSED);
    static_assert(+TriggerBarrierState::Opened == proto::TriggerBarrierState::OPENED);

    return static_cast<TriggerBarrierState>(barrierState);
}
