#include "SignalStateUkSemaphore.h"

#include "SignalStateUkSemaphore.pb.h"

SignalStateUkSemaphore::PartialState
SignalStateUkSemaphore::toPartialState() const
{
    PartialState partialState;
    partialState.m_mainAspectState = m_mainAspectState;
    partialState.m_manuallyRequestedMainAspectState = m_manuallyRequestedMainAspectState;
    partialState.m_arm = m_arm;
    return partialState;
}

std::unique_ptr<SignalState>
SignalStateUkSemaphore::clone() const
{
    return std::make_unique<SignalStateUkSemaphore>(*this);
}

CommonCabinSignalColor
SignalStateUkSemaphore::commonCabinSignalColor() const
{
    auto const state = m_manuallyRequestedMainAspectState
        .value_or(m_mainAspectState
            .value_or(UkSemaphoreAspectState::Off));

    switch (state)
    {
    case UkSemaphoreAspectState::Closed:
        return CommonCabinSignalColor::Red;
    case UkSemaphoreAspectState::Causion:
        return CommonCabinSignalColor::Yellow;
    case UkSemaphoreAspectState::Opened:
        return CommonCabinSignalColor::Green;
    }

    return CommonCabinSignalColor::Off;
}

bool
SignalStateUkSemaphore::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateUkSemaphore::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.m_mainAspectState && *partialState.m_mainAspectState != m_mainAspectState)
    {
        m_mainAspectState = *partialState.m_mainAspectState;
        modified = true;
    }

    if (partialState.m_manuallyRequestedMainAspectState &&
        *partialState.m_manuallyRequestedMainAspectState != m_manuallyRequestedMainAspectState)
    {
        m_manuallyRequestedMainAspectState = *partialState.m_manuallyRequestedMainAspectState;
        modified = true;
    }

    if (partialState.m_arm && m_arm.compare(*partialState.m_arm))
    {
        m_arm = *partialState.m_arm;
        modified = true;
    }

    return modified;
}

std::optional<UkSemaphoreAspectState>
SignalStateUkSemaphore::actualOrRequestedMainAspectState() const
{
    return m_mainAspectState.has_value() ? m_mainAspectState : m_manuallyRequestedMainAspectState;
}

/*============================= PartialState ============================*/
SignalStateUkSemaphore::PartialState::PartialState(std::string const& serialized)
{
    proto::SemaphoreStateUk protoState;
    if (!protoState.ParseFromString(serialized))
        return;

    fromProtoToUkSemaphoreAspectState(protoState.arm(), m_arm);
    fromProtoToUkSemaphoreAspectState(protoState.main_aspect_state(), m_mainAspectState);
    fromProtoToUkSemaphoreAspectState(
        protoState.manually_requested_main_aspect_state(), m_manuallyRequestedMainAspectState);
}

std::optional<UkSemaphoreAspectState>
SignalStateUkSemaphore::PartialState::fromIntToUkSemaphoreAspectState(int state) const
{
    return static_cast<UkSemaphoreAspectState>(state);
}

int
SignalStateUkSemaphore::PartialState::toUkSemaphoreAspectStateProto(
    UkSemaphoreAspectState aspectState) const
{
    return +aspectState;
}

int
SignalStateUkSemaphore::PartialState::toUkSemaphoreAspectStateProto(
    std::optional<UkSemaphoreAspectState> const& aspectState) const
{
    return aspectState ? 1 + toUkSemaphoreAspectStateProto(*aspectState) : 0;
}

int
SignalStateUkSemaphore::PartialState::toUkSemaphoreAspectStateProto(
    std::optional<std::optional<UkSemaphoreAspectState>> const& aspectState) const
{
    return aspectState ? 1 + toUkSemaphoreAspectStateProto(*aspectState) : 0;
}

std::string
SignalStateUkSemaphore::PartialState::toUkSemaphoreAspectStateProto(
    std::optional<std::string> const& arm) const
{
    return arm ? *arm : std::string();
}

void
SignalStateUkSemaphore::PartialState::fromProtoToUkSemaphoreAspectState(
    int protoVal, std::optional<UkSemaphoreAspectState>& target) const
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = fromIntToUkSemaphoreAspectState(protoVal - 1);
    }
}

void
SignalStateUkSemaphore::PartialState::fromProtoToUkSemaphoreAspectState(
    int protoVal, std::optional<std::optional<UkSemaphoreAspectState>>& target) const
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target.emplace();
        fromProtoToUkSemaphoreAspectState(protoVal - 1, *target);
    }
}

void
SignalStateUkSemaphore::PartialState::fromProtoToUkSemaphoreAspectState(
    std::string const& protoVal, std::optional<std::string>& target) const
{
    target = protoVal.empty() ? std::optional<std::string>() : protoVal;
}

std::string
SignalStateUkSemaphore::PartialState::serialize() const
{
    proto::SemaphoreStateUk protoState;

    protoState.set_arm(toUkSemaphoreAspectStateProto(m_arm));
    protoState.set_main_aspect_state(toUkSemaphoreAspectStateProto(m_mainAspectState));
    protoState.set_manually_requested_main_aspect_state(
        toUkSemaphoreAspectStateProto(m_manuallyRequestedMainAspectState));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}
