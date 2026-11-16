#include "SignalStateLegacy.h"

#include "UkMainAspectState.h"
#include "SignalStateSimple.pb.h"

SignalStateLegacy::PartialState
SignalStateLegacy::toPartialState() const
{
    PartialState partialState;
    partialState.legacyState = m_legacyState;
    partialState.symbolicIndicator = m_symbolicIndicator;
    partialState.manualSymbolicIndicator = m_manualSymbolicIndicator;

    return partialState;
}

std::unique_ptr<SignalState>
SignalStateLegacy::clone() const
{
    return std::make_unique<SignalStateLegacy>(*this);
}

bool
SignalStateLegacy::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateLegacy::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.legacyState && *partialState.legacyState != m_legacyState)
    {
        m_legacyState = *partialState.legacyState;
        modified = true;
    }

    if (partialState.symbolicIndicator != m_symbolicIndicator)
    {
        m_symbolicIndicator = partialState.symbolicIndicator;
        modified = true;
    }

    if (partialState.manualSymbolicIndicator &&
        *partialState.manualSymbolicIndicator != m_manualSymbolicIndicator)
    {
        m_manualSymbolicIndicator = *partialState.manualSymbolicIndicator;
        modified = true;
    }

    return modified;
}

bool
SignalStateLegacy::noFrequency() const
{
    return false;
}

bool
SignalStateLegacy::absoluteStop() const
{
    return false;
}

RuMetroAspectState
SignalStateLegacy::aspectState() const
{
    return ruMetroAspectStateFromLegacySignalLightState(m_legacyState);
}

CommonCabinSignalColor
SignalStateLegacy::commonCabinSignalColor() const
{
    return commonCabinSignalColorFromLegacySignalLightState(m_legacyState);
}

RuMetroAlsCode
SignalStateLegacy::alsCode() const
{
    return ruMetroAlsCodeFromLegacySignalLightState(m_legacyState);
}

bool
SignalStateLegacy::invitationActive() const
{
    return m_legacyState != LegacySignalLightState::Off &&
        m_legacyState != LegacySignalLightState::Err01 &&
        m_legacyState != LegacySignalLightState::Red &&
        m_legacyState != LegacySignalLightState::RedWhite;
}

/*============================= PartialState ============================*/

SignalStateLegacy::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateSimple protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.state(), legacyState);
    symbolicIndicatorFromProto(protoState.symbolic_indicator(), symbolicIndicator);
    manualSymbolicIndicatorFromProto(protoState.manual_symbolic_indicator(),
        manualSymbolicIndicator);
}

std::string
SignalStateLegacy::PartialState::serialize() const
{
    proto::SignalStateSimple protoState;
    protoState.set_state(toProto(legacyState));
    protoState.set_symbolic_indicator(symbolicIndicatorToProto(symbolicIndicator));
    protoState.set_manual_symbolic_indicator(
        manualSymbolicIndicatorToProto(manualSymbolicIndicator));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}
