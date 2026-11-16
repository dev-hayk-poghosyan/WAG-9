#include "SignalStateRuExit.h"

#include "RuSymbolDisplayState.h"

#include "SignalStateRuExit.pb.h"


CommonCabinSignalColor
SignalStateRuExit::commonCabinSignalColor(RuExitMainAspectState state)
{
    switch (state)
    {
    case RuExitMainAspectState::Green:
        return CommonCabinSignalColor::Green;
    case RuExitMainAspectState::Yellow:
        return CommonCabinSignalColor::Yellow;
    case RuExitMainAspectState::DoubleYellow:
        return CommonCabinSignalColor::Yellow;
    case RuExitMainAspectState::DoubleYellowTopFlashing:
        return CommonCabinSignalColor::Yellow;
    default:
        return CommonCabinSignalColor::Red;
    }
}

std::vector<RuExitMainAspectState>
SignalStateRuExit::parseAspectState(const std::string& state)
{
    return std::vector<RuExitMainAspectState>();
}

SignalStateRuExit::PartialState
SignalStateRuExit::toPartialState() const
{
    PartialState partialState;
    partialState.mainAspectState = m_mainAspectState;
    partialState.manuallyRequestedMainAspectState = m_manuallyRequestedMainAspectState;
    partialState.shuntingWhiteRequested = m_shuntingWhiteRequested;
    partialState.symbolDisplays = m_symbolDisplays.toPartialState();

    return partialState;
}

std::unique_ptr<SignalState>
SignalStateRuExit::clone() const
{
    return std::make_unique<SignalStateRuExit>(*this);
}

bool
SignalStateRuExit::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateRuExit::mergePartialState(PartialState const& partialState)
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

    if (partialState.shuntingWhiteRequested &&
        *partialState.shuntingWhiteRequested != m_shuntingWhiteRequested)
    {
        m_shuntingWhiteRequested = *partialState.shuntingWhiteRequested;
        modified = true;
    }

    modified = m_symbolDisplays.mergePartialState(partialState.symbolDisplays) || modified;

    return modified;
}

std::optional<RuExitMainAspectState>
SignalStateRuExit::actualOrRequestedMainAspectState() const
{
    return m_mainAspectState ? m_mainAspectState : m_manuallyRequestedMainAspectState;
}

RuSymbolDisplayState
SignalStateRuExit::symbolDispayValue(unsigned int index) const
{
    return m_symbolDisplays.symbolDispayValue(index);
}

std::optional<RuExitMainAspectState>
SignalStateRuExit::visualMainAspectState() const
{
    auto const mainAspectState = actualOrRequestedMainAspectState();

    if (mainAspectState == RuExitMainAspectState::Red &&
        m_shuntingWhiteRequested == BooleanAspectState::On)
    {
        // Shunting White suppresses main aspect Red.
        return std::nullopt;
    }
    else
    {
        return mainAspectState;
    }
}

BooleanAspectState
SignalStateRuExit::visualShuntingWhiteState() const
{
    if (actualOrRequestedMainAspectState() == RuExitMainAspectState::Red)
    {
        return m_shuntingWhiteRequested;
    }
    else
    {
        return BooleanAspectState::Off;
    }
}

CommonCabinSignalColor
SignalStateRuExit::commonCabinSignalColor() const
{
    if (m_mainAspectState)
    {
        return commonCabinSignalColor(*m_mainAspectState);
    }

    return CommonCabinSignalColor::White;
}


/*============================= PartialState ============================*/

SignalStateRuExit::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateRuExit protoState;
    if (!protoState.ParseFromString(serialized))
    {
        return;
    }

    fromProto(protoState.main_aspect_state(), mainAspectState);
    fromProto(protoState.manually_requested_main_aspect_state(), manuallyRequestedMainAspectState);
    fromProto(protoState.shunting_white_requested(), shuntingWhiteRequested);
    symbolDisplays.fromProto(protoState.symbol_display_1(), 0);
    symbolDisplays.fromProto(protoState.symbol_display_2(), 1);
}

std::string
SignalStateRuExit::PartialState::serialize() const
{
    proto::SignalStateRuExit protoState;
    protoState.set_main_aspect_state(toProto(mainAspectState));
    protoState.set_manually_requested_main_aspect_state(toProto(manuallyRequestedMainAspectState));
    protoState.set_shunting_white_requested(toProto(shuntingWhiteRequested));
    protoState.set_symbol_display_1(symbolDisplays.toProto(0));
    protoState.set_symbol_display_2(symbolDisplays.toProto(1));

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}

bool
SignalStateRuExit::PartialState::setSymbolDisplayState(
    unsigned int index, RuSymbolDisplayState value)
{
    return symbolDisplays.setSymbolDispayValue(index, value);
}
