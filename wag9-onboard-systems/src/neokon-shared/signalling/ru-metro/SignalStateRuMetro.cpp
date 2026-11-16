#include "SignalStateRuMetro.h"
#include "RuMetroAspectState.h"
#include "SignalStateRuMetro.pb.h"

SignalStateRuMetro::PartialState
SignalStateRuMetro::toPartialState() const
{
    PartialState partialState;
    partialState.m_mainAspectState = m_mainAspectState;
    partialState.m_manuallyRequestedMainAspectState = m_manuallyRequestedMainAspectState;
    partialState.m_autoBlocking = m_autoBlocking;
    partialState.m_absoluteStop = m_absoluteStop;
    partialState.m_alsFrequency = m_alsFrequency;
    partialState.symbolDisplays = m_symbolDisplays.toPartialState();
    return partialState;
}

CommonCabinSignalColor
SignalStateRuMetro::commonCabinSignalColor(RuMetroAspectState state)
{
    switch (state)
    {
    case RuMetroAspectState::Green:
    case RuMetroAspectState::GreenMore:
        return CommonCabinSignalColor::Green;
    case RuMetroAspectState::Red:
    case RuMetroAspectState::RedYellow:
        return CommonCabinSignalColor::Red;
    default:
        return CommonCabinSignalColor::Yellow;
    }
}

std::vector<RuMetroAspectState>
SignalStateRuMetro::parseAspectState(const std::string& state)
{
    std::vector<RuMetroAspectState> result;
    if (state.compare("IW") == 0)
    {
        result.push_back(RuMetroAspectState::InvitationWhite);
    }
    else if (state.compare("W") == 0)
    {
        result.push_back(RuMetroAspectState::White);
    }
    else if (state.compare("YY") == 0)
    {
        result.push_back(RuMetroAspectState::DoubleYellow);
        result.push_back(RuMetroAspectState::DoubleYellowTopFlashing);
    }
    else if (state.compare("B") == 0)
    {
        result.push_back(RuMetroAspectState::Blue);
    }

    return result;
}

std::unique_ptr<SignalState>
SignalStateRuMetro::clone() const
{
    return std::make_unique<SignalStateRuMetro>(*this);
}

bool
SignalStateRuMetro::mergeSerializedPartialState(std::string const& serialized)
{
    return mergePartialState(PartialState(serialized));
}

bool
SignalStateRuMetro::mergePartialState(PartialState const& partialState)
{
    bool modified = false;

    bool mainAspectIsNotRed = false;

    if (partialState.m_mainAspectState && *partialState.m_mainAspectState != m_mainAspectState &&
        m_absoluteStop != true)
    {
        m_mainAspectState = *partialState.m_mainAspectState;

        mainAspectIsNotRed = m_mainAspectState != RuMetroAspectState::Red;

        modified = true;
    }

    if (partialState.m_manuallyRequestedMainAspectState &&
        *partialState.m_manuallyRequestedMainAspectState != m_manuallyRequestedMainAspectState)
    {
        m_manuallyRequestedMainAspectState = *partialState.m_manuallyRequestedMainAspectState;
        modified = true;
    }

    if (partialState.m_autoBlocking && *partialState.m_autoBlocking != m_autoBlocking)
    {
        m_autoBlocking = *partialState.m_autoBlocking;
        modified = true;
    }

    if (m_absoluteStop && mainAspectIsNotRed && !partialState.m_absoluteStop.value_or(false))
    {
        // Reset absolute stop mode if the signal value is changed to non-red aspect state
        // with no forcing to enable the absolute stop.
        m_absoluteStop = false;
        modified = true;
    }

    if (partialState.m_absoluteStop && *partialState.m_absoluteStop != m_absoluteStop)
    {
        m_absoluteStop = *partialState.m_absoluteStop;
        modified = true;
    }

    if (modified && m_absoluteStop == true)
    {
        m_mainAspectState = RuMetroAspectState::Red;
    }

    if (partialState.m_alsFrequency && *partialState.m_alsFrequency != m_alsFrequency)
    {
        m_alsFrequency = *partialState.m_alsFrequency;
        modified = true;
    }

    modified = m_symbolDisplays.mergePartialState(partialState.symbolDisplays) || modified;

    return modified;
}

std::optional<RuMetroAspectState>
SignalStateRuMetro::actualOrRequestedMainAspectState() const
{
    return m_mainAspectState ? m_mainAspectState : m_manuallyRequestedMainAspectState;
}

bool SignalStateRuMetro::kguDisplayState() const
{
    return kguDispayValue().value_or(false);
}

std::optional<bool>
SignalStateRuMetro::kguDispayValue() const
{
    return m_symbolDisplays.kguDispayValue();
}

RuSymbolDisplayState
SignalStateRuMetro::symbolDispayValue(unsigned int index) const
{
    return m_symbolDisplays.symbolDispayValue(index);
}

bool
SignalStateRuMetro::isFlashingValue(RuMetroAspectState state) const
{
    switch (state)
    {
    case RuMetroAspectState::DoubleYellowTopFlashing:
    case RuMetroAspectState::FlashingYellow:
    case RuMetroAspectState::InvitationWhite:
        return true;
    default:
        return false;
    }
}

CommonCabinSignalColor SignalStateRuMetro::commonCabinSignalColor() const
{
    return m_mainAspectState ?
            commonCabinSignalColor(*m_mainAspectState) : CommonCabinSignalColor::Off;
}

bool
SignalStateRuMetro::absoluteStop() const
{
    return m_absoluteStop;
}

RuMetroAspectState
SignalStateRuMetro::aspectState() const
{
    if (m_alsState)
    {
        return *m_alsState;
    }

    if (auto const aspectState = actualOrRequestedMainAspectState())
    {
        return *aspectState;
    }

    return RuMetroAspectState::Red;
}

std::string
SignalStateRuMetro::alsCodeStr() const
{
    return ruMetroAlsCodeToStr(alsCode());
}

RuMetroAlsCode
SignalStateRuMetro::alsCode() const
{
    if (m_noFrequency)
    {
        return RuMetroAlsCode::NoFrequency0;
    }

    if (m_alsFrequency)
    {
        return *m_alsFrequency;
    }

    if (m_absoluteStop)
    {
        return RuMetroAlsCode::AbsoluteStop0;
    }

    RuMetroAspectState state =
        m_alsState.value_or(actualOrRequestedMainAspectState().value_or(RuMetroAspectState::Red));

    if (m_limit->noFrequencyFor(state))
    {
        return RuMetroAlsCode::NoFrequency0;
    }

    if (state != RuMetroAspectState::Red &&
        state != RuMetroAspectState::InvitationWhite &&
        state != RuMetroAspectState::RedYellow)
    {
        std::optional<int> limit = m_limit->speedLimitFor(state);
        if (limit)
        {
            if (*limit >= 80)
                return state == RuMetroAspectState::GreenMore ?
                    RuMetroAlsCode::Green80 : RuMetroAlsCode::Green70;
            if (*limit >= 75)
                return RuMetroAlsCode::Green75;
            if (*limit >= 70)
                return RuMetroAlsCode::Green70;
            if (*limit >= 60)
                return RuMetroAlsCode::Green60;
            if (*limit >= 40)
                return RuMetroAlsCode::Yellow40;

            return RuMetroAlsCode::Red0;
        }
    }

    return m_specFrequency ?
        alsCodeSpecial(state) : alsCodeCommon(state);
}

bool
SignalStateRuMetro::invitationActive() const
{
    const auto state = actualOrRequestedMainAspectState().value_or(RuMetroAspectState::Undefined);
    switch (state)
    {
    case RuMetroAspectState::Undefined:
    case RuMetroAspectState::Red:
    case RuMetroAspectState::RedYellow:
        return false;
    default:
        return true;
    } 
}

bool
SignalStateRuMetro::noFrequency() const
{
    return m_noFrequency;
}

RuMetroAlsCode
SignalStateRuMetro::alsCodeCommon(RuMetroAspectState state) const
{
    switch (state)
    {
    case RuMetroAspectState::Red:
        return RuMetroAlsCode::NoFrequency0;
    case RuMetroAspectState::RedYellow:
    case RuMetroAspectState::InvitationWhite:
        return RuMetroAlsCode::Red0;
    case RuMetroAspectState::Green:
        return RuMetroAlsCode::Green70;
    case RuMetroAspectState::GreenMore:
        return RuMetroAlsCode::Green80;
    case RuMetroAspectState::YellowGreen:
        return RuMetroAlsCode::Green60;
    case RuMetroAspectState::White:
    case RuMetroAspectState::Yellow:
    case RuMetroAspectState::DoubleYellow:
    case RuMetroAspectState::DoubleYellowTopFlashing:
        return RuMetroAlsCode::Yellow40;
    default:
        return RuMetroAlsCode::NoFrequency0;
    }
}

RuMetroAlsCode
SignalStateRuMetro::alsCodeSpecial(RuMetroAspectState state) const
{
    switch (state)
    {
    case RuMetroAspectState::Red:
        return RuMetroAlsCode::NoFrequency0;
    case RuMetroAspectState::RedYellow:
    case RuMetroAspectState::InvitationWhite:
        return RuMetroAlsCode::Red0;

    case RuMetroAspectState::Green:
    case RuMetroAspectState::YellowGreen:
    case RuMetroAspectState::White:
    case RuMetroAspectState::Yellow:
    case RuMetroAspectState::DoubleYellow:
    case RuMetroAspectState::DoubleYellowTopFlashing:
        return RuMetroAlsCode::Yellow40;

    default:
        return RuMetroAlsCode::NoFrequency0;
    }
}

/*============================= PartialState ============================*/
SignalStateRuMetro::PartialState::PartialState(std::string const& serialized)
{
    proto::SignalStateRuMetro protoState;
    if (!protoState.ParseFromString(serialized))
        return;

    fromProto(protoState.main_aspect_state(), m_mainAspectState);
    fromProto(protoState.manually_requested_main_aspect_state(), m_manuallyRequestedMainAspectState);
    fromProto(protoState.auto_blocking(), m_autoBlocking);
    fromProto(protoState.absolute_stop(), m_absoluteStop);
    fromProto(protoState.als_frequency(), m_alsFrequency);
    symbolDisplays.fromProto(protoState.symbol_display_1(), 0);
    symbolDisplays.fromProto(protoState.symbol_display_2(), 1);
    symbolDisplays.fromProtoKGU(protoState.symbol_display_kgu());
}

std::optional<RuMetroAspectState>
SignalStateRuMetro::PartialState::fromIntToAspectState(int state) const
{
    return static_cast<RuMetroAspectState>(state);
}

int
SignalStateRuMetro::PartialState::toProto(RuMetroAspectState aspectState) const
{
    return +aspectState;
}

int
SignalStateRuMetro::PartialState::toProto(bool value) const
{
    return value ? 2 : 1;
}

int
SignalStateRuMetro::PartialState::toProto(
    std::optional<RuMetroAspectState> const& aspectState) const
{
    return aspectState ? 1 + toProto(*aspectState) : 0;
}

int
SignalStateRuMetro::PartialState::toProto(std::optional<bool> const& value) const
{
    return value ? toProto(*value) : 0;
}

int
SignalStateRuMetro::PartialState::toProto(
    std::optional<std::optional<RuMetroAspectState>> const& aspectState) const
{
    return aspectState ? 1 + toProto(*aspectState) : 0;
}

int
SignalStateRuMetro::PartialState::toProto(std::optional<RuMetroAlsCode> const& value) const
{
    return value ? +(*value) + 1 : 0;
}

void
SignalStateRuMetro::PartialState::fromProto(int protoVal, std::optional<bool>& target) const
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = protoVal == 2;
    }
}

void
SignalStateRuMetro::PartialState::fromProto(int protoVal, std::optional<RuMetroAlsCode>& target) const
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = static_cast<RuMetroAlsCode>(protoVal - 1);
    }
}

void
SignalStateRuMetro::PartialState::fromProto(
    int protoVal, std::optional<RuMetroAspectState>& target) const
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target = fromIntToAspectState(protoVal - 1);
    }
}

void
SignalStateRuMetro::PartialState::fromProto(
    int protoVal, std::optional<std::optional<RuMetroAspectState>>& target) const
{
    if (protoVal == 0)
    {
        target = std::nullopt;
    }
    else
    {
        target.emplace();
        fromProto(protoVal - 1, *target);
    }
}

std::string
SignalStateRuMetro::PartialState::serialize() const
{
    proto::SignalStateRuMetro protoState;
    protoState.set_main_aspect_state(toProto(m_mainAspectState));
    protoState.set_manually_requested_main_aspect_state(
        toProto(m_manuallyRequestedMainAspectState));
    protoState.set_auto_blocking(toProto(m_autoBlocking));
    protoState.set_absolute_stop(toProto(m_absoluteStop));
    protoState.set_als_frequency(toProto(m_alsFrequency));
    protoState.set_symbol_display_1(symbolDisplays.toProto(0));
    protoState.set_symbol_display_2(symbolDisplays.toProto(1));
    protoState.set_symbol_display_kgu(symbolDisplays.toProtoKGU());

    std::string serialized;
    protoState.SerializeToString(&serialized);

    return serialized;
}

bool
SignalStateRuMetro::PartialState::setSymbolDisplayState(
    unsigned int index, RuSymbolDisplayState value)
{
    return symbolDisplays.setSymbolDispayValue(index, value);
}

bool
SignalStateRuMetro::PartialState::setSymbolDisplayStateKGU(bool value)
{
    return symbolDisplays.setSymbolDispayValueKGU(value);
}
