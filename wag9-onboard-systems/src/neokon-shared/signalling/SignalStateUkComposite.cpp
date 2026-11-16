#include "SignalStateUkComposite.h"

#include "SignalStateUkComposite.pb.h"

SignalStateUkComposite::PartialState
SignalStateUkComposite::toPartialState() const
{
    PartialState partialState;
    partialState.mainAspectState = m_mainAspectState;
    partialState.litJunctionIndicatorArm = m_litJunctonIndicatorArm;
    partialState.manualMainAspectState = m_manualMainAspectState;
    partialState.manualAplsState = m_manualAplsState;
    partialState.manualRightAwayState = m_manualRightAwayState;
    partialState.symbolicIndicator = m_symbolicIndicator;
    partialState.manualSymbolicIndicator = m_manualSymbolicIndicator;

    return partialState;
}

std::unique_ptr<SignalState>
SignalStateUkComposite::clone() const
{
    return std::make_unique<SignalStateUkComposite>(*this);
}

bool
SignalStateUkComposite::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateUkComposite::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.mainAspectState && *partialState.mainAspectState != m_mainAspectState)
    {
        m_mainAspectState = *partialState.mainAspectState;
        modified = true;
    }

    if (partialState.manualMainAspectState &&
        *partialState.manualMainAspectState != m_manualMainAspectState)
    {
        m_manualMainAspectState = *partialState.manualMainAspectState;
        modified = true;
    }

    if (partialState.manualAplsState && *partialState.manualAplsState != m_manualAplsState)
    {
        m_manualAplsState = *partialState.manualAplsState;
        modified = true;
    }

    if (partialState.manualRightAwayState &&
        *partialState.manualRightAwayState != m_manualRightAwayState)
    {
        m_manualRightAwayState = *partialState.manualRightAwayState;
        modified = true;
    }

    if (partialState.litJunctionIndicatorArm &&
        *partialState.litJunctionIndicatorArm != m_litJunctonIndicatorArm)
    {
        m_litJunctonIndicatorArm = *partialState.litJunctionIndicatorArm;
        modified = true;
    }

    if (partialState.symbolicIndicator && *partialState.symbolicIndicator != m_symbolicIndicator)
    {
        m_symbolicIndicator = *partialState.symbolicIndicator;
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

CommonCabinSignalColor
SignalStateUkComposite::commonCabinSignalColor() const
{
    // This method is used by bot trains.

    if (m_mainAspectState)
    {
        switch (*m_mainAspectState)
        {
        case UkMainAspectState::Green:
            return CommonCabinSignalColor::Green;
        case UkMainAspectState::Yellow:
        case UkMainAspectState::DoubleYellow:
        case UkMainAspectState::FlashingYellow:
        case UkMainAspectState::FlashingDoubleYellow:
            return CommonCabinSignalColor::Yellow;
        case UkMainAspectState::Red:
            return CommonCabinSignalColor::RedYellow;
        }
    }

    return CommonCabinSignalColor::Off;
}


/*============================== PartialState ============================*/

SignalStateUkComposite::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateUkComposite protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.main_aspect_state(), mainAspectState);
    fromProto(protoState.manual_main_aspect_state(), manualMainAspectState);
    fromProto(protoState.manual_apls_state(), manualAplsState);
    fromProto(protoState.manual_right_away_state(), manualRightAwayState);
    fromProto(protoState.lit_junction_indicator_arm(), litJunctionIndicatorArm);
    symbolicIndicatorFromProto(protoState.symbolic_indicator(), symbolicIndicator);
    manualSymbolicIndicatorFromProto(protoState.manual_symbolic_indicator(),
        manualSymbolicIndicator);
}

std::string
SignalStateUkComposite::PartialState::serialize() const
{
    proto::SignalStateUkComposite protoState;
    protoState.set_main_aspect_state(toProto(mainAspectState));
    protoState.set_manual_main_aspect_state(toProto(manualMainAspectState));
    protoState.set_manual_apls_state(toProto(manualAplsState));
    protoState.set_manual_right_away_state(toProto(manualRightAwayState));
    protoState.set_lit_junction_indicator_arm(toProto(litJunctionIndicatorArm));
    protoState.set_symbolic_indicator(symbolicIndicatorToProto(symbolicIndicator));
    protoState.set_manual_symbolic_indicator(
        manualSymbolicIndicatorToProto(manualSymbolicIndicator));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}
