#include "SignalStateLegacySemiAuto.h"

#include "SignalStateSimpleSemiAuto.pb.h"

SignalStateLegacySemiAuto::PartialState
SignalStateLegacySemiAuto::toPartialState() const
{
    PartialState partialState;
    partialState.actualState = m_actualState;
    partialState.manualStateRequest = m_manualStateRequest;

    return partialState;
}

std::unique_ptr<SignalState>
SignalStateLegacySemiAuto::clone() const
{
    return std::make_unique<SignalStateLegacySemiAuto>(*this);
}

bool
SignalStateLegacySemiAuto::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateLegacySemiAuto::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.actualState && *partialState.actualState != m_actualState)
    {
        m_actualState = *partialState.actualState;
        modified = true;
    }

    if (partialState.manualStateRequest && *partialState.manualStateRequest != m_manualStateRequest)
    {
        m_manualStateRequest = *partialState.manualStateRequest;
        modified = true;
    }

    return modified;
}

bool
SignalStateLegacySemiAuto::noFrequency() const
{
    return false;
}

bool
SignalStateLegacySemiAuto::absoluteStop() const
{
    return false;
}

RuMetroAspectState
SignalStateLegacySemiAuto::aspectState() const
{
    if (m_manualStateRequest)
    {
        return ruMetroAspectStateFromLegacySignalLightState(*m_manualStateRequest);
    }
    else if (m_actualState)
    {
        return ruMetroAspectStateFromLegacySignalLightState(*m_actualState);
    }
    return RuMetroAspectState::Red;
}

CommonCabinSignalColor
SignalStateLegacySemiAuto::commonCabinSignalColor() const
{
    if (!m_actualState)
    {
        return CommonCabinSignalColor::Off;
    }
    else
    {
        return commonCabinSignalColorFromLegacySignalLightState(*m_actualState);
    }
}

RuMetroAlsCode
SignalStateLegacySemiAuto::alsCode() const
{
    if (!m_actualState)
    {
        return RuMetroAlsCode::NoFrequency0;
    }
    else
    {
        return ruMetroAlsCodeFromLegacySignalLightState(*m_actualState);
    }
}

bool
SignalStateLegacySemiAuto::invitationActive() const
{
    return m_actualState != LegacySignalLightState::Off &&
        m_actualState != LegacySignalLightState::Err01 &&
        m_actualState != LegacySignalLightState::Red &&
        m_actualState != LegacySignalLightState::RedWhite;
}

/*================================ PartialState ==========================*/

SignalStateLegacySemiAuto::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateSimpleSemiAuto protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.actual_state(), actualState);
    fromProto(protoState.manual_state_request(), manualStateRequest);
}

std::string
SignalStateLegacySemiAuto::PartialState::serialize() const
{
    proto::SignalStateSimpleSemiAuto protoState;
    protoState.set_actual_state(toProto(actualState));
    protoState.set_manual_state_request(toProto(manualStateRequest));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}
