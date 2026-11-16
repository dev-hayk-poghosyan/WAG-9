#include "SignalStateRuSymbolDisplay.h"

#include "RuSymbolDisplayState.h"

bool
SignalStateRuSymbolDisplay::mergePartialState(const SignalStateRuSymbolDisplay::PartialState& state)
{
    bool modified = false;

    if (state.symbolDisplay1 &&
        *state.symbolDisplay1 != m_symbolDisplay1)
    {
        m_symbolDisplay1 = *state.symbolDisplay1;
        modified = true;
    }

    if (state.symbolDisplay2 &&
        *state.symbolDisplay2 != m_symbolDisplay2)
    {
        m_symbolDisplay2 = *state.symbolDisplay2;
        modified = true;
    }

    if (state.symbolDisplayKgu &&
        *state.symbolDisplayKgu != m_symbolDisplayKgu)
    {
        m_symbolDisplayKgu = *state.symbolDisplayKgu;
        modified = true;
    }

    return modified;
}

SignalStateRuSymbolDisplay::PartialState
SignalStateRuSymbolDisplay::toPartialState() const
{
    SignalStateRuSymbolDisplay::PartialState partialState;
    partialState.symbolDisplay1 = m_symbolDisplay1;
    partialState.symbolDisplay2 = m_symbolDisplay2;
    partialState.symbolDisplayKgu = m_symbolDisplayKgu;
    return partialState;
}

RuSymbolDisplayState
SignalStateRuSymbolDisplay::symbolDispayValue(unsigned int index) const
{
    if (index == 0)
    {
        return m_symbolDisplay1.value_or(RuSymbolDisplayState::Off);
    }

    if (index == 1)
    {
        return m_symbolDisplay2.value_or(RuSymbolDisplayState::Off);
    }

    assert("Incorrect display index: " + index);

    return RuSymbolDisplayState::Error;
}

std::optional<bool>
SignalStateRuSymbolDisplay::kguDispayValue() const
{
    return m_symbolDisplayKgu;
}

/*============================= PartialState ============================*/

void
SignalStateRuSymbolDisplay::PartialState::fromProto(int protoVal, unsigned int index)
{
    std::optional<RuSymbolDisplayState> target;
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = static_cast<RuSymbolDisplayState>(protoVal - 1);
    }

    if (index == 0)
    {
        symbolDisplay1 = target;
    }
    else if (index == 1)
    {
        symbolDisplay2 = target;
    }
    else
    {
        assert("Incorrect display index: " + index);
    }
}

void
SignalStateRuSymbolDisplay::PartialState::fromProtoKGU(int protoVal)
{
    if (protoVal == 0)
    {
        symbolDisplayKgu = std::nullopt;
    }
    else 
    {
        symbolDisplayKgu = protoVal > 1;
    }
}

bool
SignalStateRuSymbolDisplay::PartialState::setSymbolDispayValue(unsigned int index,
    const std::optional<RuSymbolDisplayState>& value)
{
    std::optional<RuSymbolDisplayState> cur;

    if (index == 0)
    {
        cur = symbolDisplay1;
        symbolDisplay1 = value;
    }
    else if (index == 1)
    {
        cur = symbolDisplay2;
        symbolDisplay2 = value;
    }
    else
    {
        assert("Incorrect display index: " + index);
    }

    return cur.value_or(RuSymbolDisplayState::Error) != value.value_or(RuSymbolDisplayState::Error);
}

bool
SignalStateRuSymbolDisplay::PartialState::setSymbolDispayValueKGU(bool value)
{
    std::optional<bool> cur = symbolDisplayKgu;
    symbolDisplayKgu = value;
    return !cur || cur.value() != value;
}

int
SignalStateRuSymbolDisplay::PartialState::toProto(unsigned int index) const
{
    std::optional<RuSymbolDisplayState> cur;

    if (index == 0)
    {
        cur = symbolDisplay1;
    }
    else if (index == 1)
    {
        cur = symbolDisplay2;
    }
    else
    {
        assert("Incorrect display index: " + index);
    }

    return cur == std::nullopt ? 0 : static_cast<int>(*cur) + 1;
}

int
SignalStateRuSymbolDisplay::PartialState::toProtoKGU() const
{
    if (!symbolDisplayKgu)
    {
        return 0;
    }
    return *symbolDisplayKgu == false ? 1 : 2;
}
