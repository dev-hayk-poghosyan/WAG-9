#include "SignalStateUkBannerRepeater.h"

#include "SignalStateUkBannerRepeater.pb.h"

std::unique_ptr<SignalState>
SignalStateUkBannerRepeater::clone() const
{
    return std::make_unique<SignalStateUkBannerRepeater>(*this);
}

bool
SignalStateUkBannerRepeater::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateUkBannerRepeater::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.primaryBannerState &&
        *partialState.primaryBannerState != m_primaryBannerState)
    {
        m_primaryBannerState = *partialState.primaryBannerState;
        modified = true;
    }

    if (partialState.secondaryBannerState &&
        *partialState.secondaryBannerState != m_secondaryBannerState)
    {
        m_secondaryBannerState = *partialState.secondaryBannerState;
        modified = true;
    }

    return modified;
}


/*============================== PartialState ============================*/

SignalStateUkBannerRepeater::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateUkBannerRepeater protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.primary_banner_state(), primaryBannerState);
    fromProto(protoState.secondary_banner_state(), secondaryBannerState);
}

std::string
SignalStateUkBannerRepeater::PartialState::serialize() const
{
    proto::SignalStateUkBannerRepeater protoState;
    protoState.set_primary_banner_state(toProto(primaryBannerState));
    protoState.set_secondary_banner_state(toProto(secondaryBannerState));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}
