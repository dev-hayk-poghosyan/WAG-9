#include "SignalStateUkPreliminaryRouteIndicator.h"

#include "SignalStateSimple.pb.h"

std::unique_ptr<SignalState>
SignalStateUkPreliminaryRouteIndicator::clone() const
{
    return std::make_unique<SignalStateUkPreliminaryRouteIndicator>(*this);
}

bool
SignalStateUkPreliminaryRouteIndicator::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateUkPreliminaryRouteIndicator::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.indicatorState &&
        *partialState.indicatorState != m_indicatorState)
    {
        m_indicatorState = *partialState.indicatorState;
        modified = true;
    }

    return modified;
}

/*============================== PartialState ============================*/

SignalStateUkPreliminaryRouteIndicator::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateSimple protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.state(), indicatorState);
}

std::string
SignalStateUkPreliminaryRouteIndicator::PartialState::serialize() const
{
    proto::SignalStateSimple protoState;
    protoState.set_state(toProto(indicatorState));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}

