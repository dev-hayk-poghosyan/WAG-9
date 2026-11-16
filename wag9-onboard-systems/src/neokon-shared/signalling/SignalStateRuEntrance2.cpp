#include "SignalStateRuEntrance2.h"

#include "RuSymbolDisplayState.h"

#include "SignalStateRuEntrance2.pb.h"

SignalStateRuEntrance2::PartialState
SignalStateRuEntrance2::toPartialState() const
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
SignalStateRuEntrance2::clone() const
{
    return std::make_unique<SignalStateRuEntrance2>(*this);
}

bool
SignalStateRuEntrance2::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateRuEntrance2::mergePartialState(PartialState const& partialState)
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
SignalStateRuEntrance2::commonCabinSignalColor(RuEntrance2MainAspectState state)
{
    switch (state)
    {
    case RuEntrance2MainAspectState::DoubleYellow:
        return CommonCabinSignalColor::Green;
    default:
        return CommonCabinSignalColor::Red;
    }
}
std::vector<RuEntrance2MainAspectState>
SignalStateRuEntrance2::parseAspectState(const std::string& state)
{
    return std::vector<RuEntrance2MainAspectState>();
}

CommonCabinSignalColor
SignalStateRuEntrance2::commonCabinSignalColor() const
{
    if (m_mainAspectState)
    {
        return commonCabinSignalColor(*m_mainAspectState);
    }

    return CommonCabinSignalColor::White;
}

std::optional<RuEntrance2MainAspectState>
SignalStateRuEntrance2::actualOrRequestedMainAspectState() const
{
    return m_mainAspectState ? m_mainAspectState : m_manuallyRequestedMainAspectState;
}

RuSymbolDisplayState
SignalStateRuEntrance2::symbolDispayValue(unsigned int index) const
{
    return m_symbolDisplays.symbolDispayValue(index);
}

RuEntrance2MainAspectVisualState
SignalStateRuEntrance2::visualMainAspectState() const
{
    auto const mainAspectState = actualOrRequestedMainAspectState();

    if (mainAspectState == RuEntrance2MainAspectState::Red &&
        m_shuntingWhiteRequested == BooleanAspectState::On)
    {
        // Shunting White suppresses main aspect Red.
        return RuEntrance2MainAspectVisualState();
    }
    else
    {
        return RuEntrance2MainAspectVisualState::forState(mainAspectState);
    }
}

LampMode
SignalStateRuEntrance2::visualInvitationalWhiteState() const
{
    if (m_invitationalWhiteRequested == BooleanAspectState::On &&
        actualOrRequestedMainAspectState() == RuEntrance2MainAspectState::Red)
    {
        return LampMode::Flashing;
    }
    else
    {
        return LampMode::Off;
    }
}

LampMode
SignalStateRuEntrance2::visualShuntingWhiteState() const
{
    if (m_shuntingWhiteRequested == BooleanAspectState::On &&
        m_invitationalWhiteRequested == BooleanAspectState::Off &&
        actualOrRequestedMainAspectState() == RuEntrance2MainAspectState::Red)
    {
        return LampMode::On;
    }
    else
    {
        return LampMode::Off;
    }
}


/*============================= PartialState ============================*/

SignalStateRuEntrance2::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateRuEntrance2 protoState;
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
SignalStateRuEntrance2::PartialState::serialize() const
{
    proto::SignalStateRuEntrance2 protoState;
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
SignalStateRuEntrance2::PartialState::setSymbolDisplayState(
    unsigned int index, RuSymbolDisplayState value)
{
    return symbolDisplays.setSymbolDispayValue(index, value);
}
