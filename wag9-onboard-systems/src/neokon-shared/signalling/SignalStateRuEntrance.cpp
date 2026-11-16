#include "SignalStateRuEntrance.h"

#include "RuSymbolDisplayState.h"

#include "SignalStateRuEntrance.pb.h"

CommonCabinSignalColor
SignalStateRuEntrance::commonCabinSignalColor(RuEntranceMainAspectState state)
{
    switch (state)
    {
    case RuEntranceMainAspectState::Green:
        return CommonCabinSignalColor::Green;
    case RuEntranceMainAspectState::Yellow:
        return CommonCabinSignalColor::Yellow;
    case RuEntranceMainAspectState::DoubleYellow:
        return CommonCabinSignalColor::Yellow;
    case RuEntranceMainAspectState::FlashingYellow:
        return CommonCabinSignalColor::Green; // Not obvious but correct.
    case RuEntranceMainAspectState::DoubleYellowTopFlashing:
        return CommonCabinSignalColor::Yellow;
    default:
        return CommonCabinSignalColor::Red;
    }
}

std::vector<RuEntranceMainAspectState>
SignalStateRuEntrance::parseAspectState(const std::string& state)
{
    return std::vector<RuEntranceMainAspectState>();
}

SignalStateRuEntrance::PartialState
SignalStateRuEntrance::toPartialState() const
{
    PartialState partialState;
    partialState.mainAspectState = m_mainAspectState;
    partialState.manuallyRequestedMainAspectState = m_manuallyRequestedMainAspectState;
    partialState.invitationalWhiteRequested = m_invitationalWhiteRequested;
    partialState.shuntingWhiteRequested = m_shuntingWhiteRequested;
    partialState.symbolDisplays = m_symbolDisplays.toPartialState();

    return partialState;
}

std::unique_ptr<SignalState>
SignalStateRuEntrance::clone() const
{
    return std::make_unique<SignalStateRuEntrance>(*this);
}

bool
SignalStateRuEntrance::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateRuEntrance::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    if (partialState.mainAspectState && *partialState.mainAspectState != m_mainAspectState)
    {
        m_mainAspectState = *partialState.mainAspectState;
        modified = true;
    }

    if (partialState.manuallyRequestedMainAspectState &&
        *partialState.manuallyRequestedMainAspectState != m_manuallyRequestedMainAspectState)
    {
        m_manuallyRequestedMainAspectState = *partialState.manuallyRequestedMainAspectState;
        modified = true;
    }

    if (partialState.invitationalWhiteRequested &&
        *partialState.invitationalWhiteRequested != m_invitationalWhiteRequested)
    {
        m_invitationalWhiteRequested = *partialState.invitationalWhiteRequested;
        modified = true;
    }

    if (partialState.shuntingWhiteRequested &&
        *partialState.shuntingWhiteRequested != m_shuntingWhiteRequested)
    {
        m_shuntingWhiteRequested = *partialState.shuntingWhiteRequested;
        modified = true;
    }

    modified = m_symbolDisplays.mergePartialState(partialState.symbolDisplays) || modified;

    return modified;
}

CommonCabinSignalColor
SignalStateRuEntrance::commonCabinSignalColor() const
{
    if (m_mainAspectState)
    {
        return commonCabinSignalColor(*m_mainAspectState);
    }

    return CommonCabinSignalColor::White;
}

SignalStateAspectRuEntrance::RepeaterState
SignalStateRuEntrance::repeaterState() const
{
    switch (m_mainAspectState.value_or(RuEntranceMainAspectState::Red))
    {
    case RuEntranceMainAspectState::Green:
        return RepeaterState::Green;
    case RuEntranceMainAspectState::FlashingYellow:
        return RepeaterState::YellowFlash;
    case RuEntranceMainAspectState::Red:
        return RepeaterState::Off;
    default:
        return RepeaterState::Yellow;
    }
}

std::optional<RuEntranceMainAspectState>
SignalStateRuEntrance::actualOrRequestedMainAspectState() const
{
    return m_mainAspectState ? m_mainAspectState : m_manuallyRequestedMainAspectState;
}

RuSymbolDisplayState
SignalStateRuEntrance::symbolDispayValue(unsigned int index) const
{
    return m_symbolDisplays.symbolDispayValue(index);
}

RuEntranceMainAspectVisualState
SignalStateRuEntrance::visualMainAspectState() const
{
    auto const mainAspectState = actualOrRequestedMainAspectState();

    if (mainAspectState == RuEntranceMainAspectState::Red &&
        m_shuntingWhiteRequested == BooleanAspectState::On)
    {
        // Shunting White suppresses main aspect Red.
        return RuEntranceMainAspectVisualState();
    }
    else
    {
        return RuEntranceMainAspectVisualState::forState(mainAspectState);
    }
}

LampMode
SignalStateRuEntrance::visualInvitationalWhiteState() const
{
    if (m_invitationalWhiteRequested == BooleanAspectState::On &&
        actualOrRequestedMainAspectState() == RuEntranceMainAspectState::Red)
    {
        return LampMode::Flashing;
    }
    else
    {
        return LampMode::Off;
    }
}

LampMode
SignalStateRuEntrance::visualShuntingWhiteState() const
{
    if (m_shuntingWhiteRequested == BooleanAspectState::On &&
        m_invitationalWhiteRequested == BooleanAspectState::Off &&
        actualOrRequestedMainAspectState() == RuEntranceMainAspectState::Red)
    {
        return LampMode::On;
    }
    else
    {
        return LampMode::Off;
    }
}


/*============================= PartialState ============================*/

SignalStateRuEntrance::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateRuEntrance protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.main_aspect_state(), mainAspectState);
    fromProto(protoState.manually_requested_main_aspect_state(), manuallyRequestedMainAspectState);
    fromProto(protoState.invitational_white_requested(), invitationalWhiteRequested);
    fromProto(protoState.shunting_white_requested(), shuntingWhiteRequested);
    symbolDisplays.fromProto(protoState.symbol_display_1(), 0);
    symbolDisplays.fromProto(protoState.symbol_display_2(), 1);
}

std::string
SignalStateRuEntrance::PartialState::serialize() const
{
    proto::SignalStateRuEntrance protoState;
    protoState.set_main_aspect_state(toProto(mainAspectState));
    protoState.set_manually_requested_main_aspect_state(toProto(manuallyRequestedMainAspectState));
    protoState.set_invitational_white_requested(toProto(invitationalWhiteRequested));
    protoState.set_shunting_white_requested(toProto(shuntingWhiteRequested));
    protoState.set_symbol_display_1(symbolDisplays.toProto(0));
    protoState.set_symbol_display_2(symbolDisplays.toProto(1));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}

bool
SignalStateRuEntrance::PartialState::setSymbolDisplayState(
    unsigned int index, RuSymbolDisplayState value)
{
    return symbolDisplays.setSymbolDispayValue(index, value);
}
