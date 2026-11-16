#include "SignalRuMetroSpeedLimit.h"

#include "neokon-shared/UnitConv.h"
#include "neokon-shared/signalling/ru-metro/SignalStateRuMetro.h"

SignalRuMetroSpeedLimit::SignalRuMetroSpeedLimit(
    std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits)
    : m_arsMode(arsMode(speedLimits))
    , m_alsArs(false)
    , m_routeType(m_arsMode ? defaultArsRouteType : defaultAutoblockingRouteType)
{
    if (speedLimits.empty())
    {
        return;
    }
    fillAutoblockLimits(speedLimits);
    fillArsLimits(speedLimits);
}

bool
SignalRuMetroSpeedLimit::arsMode(
    std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits) const
{
    auto const len = speedLimits.size();
    if (len < 2)
    {
        return false;
    }
    auto const& speedLimit = speedLimits[1];
    for (const auto& [serializedState, _] : speedLimit.speedByState)
    {
        const RuMetroAspectState state = aspectState(serializedState);
        if (state == RuMetroAspectState::Blue || state == RuMetroAspectState::Off)
        {
            return true;
        }
    }
    return false;
}

void
SignalRuMetroSpeedLimit::fillAutoblockLimits(
    std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits)
{
    auto const len = speedLimits.size();
    for (size_t type = 0; type < len; type++)
    {
        SpeedLimitsMap limits;
        auto const& speedLimit = speedLimits[type];
        for (const auto& [serializedState, data] : speedLimit.speedByState)
        {
            if (serializedState.empty())
            {
                continue;
            }

            RuMetroAspectState state = aspectState(serializedState);
            limits[state] = data;
        }
        m_alsLimits[type] = limits;
        if (m_arsMode && type > 0)
        {
            break;
        }
    }
}

void
SignalRuMetroSpeedLimit::fillArsLimits(
    std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits)
{
    auto const len = speedLimits.size();
    if (len < 2 || !m_arsMode)
    {
        return;
    }

    for (size_t i = 1; i < len; i++)
    {
        auto const& speedLimit = speedLimits[i];
        ArsSpeedLimitsMap limits;
        for (const auto& [serializedState, data] : speedLimit.speedByState)
        {
            const auto sections = serializedState.empty() ? 0 : data.sections.value_or(0);

            if (sections == 0)
            {
                continue;
            }
            RuMetroAspectState state = aspectState(serializedState);
            if (state == RuMetroAspectState::RedYellow)
            {
                m_alsArs = true;
            }
            const int kmh = std::round(UnitConv::ms2kmh(data.speedMs));
            const RuMetroAlsCode alsCode = ruMetroAlsCodeFromInt(kmh);
            limits[sections] = alsCode;
        }
        m_arsLimits[speedLimit.type] = limits;
    }
}

int
SignalRuMetroSpeedLimit::autoblockingRouteType() const
{
    return m_arsMode ? 0 : m_routeType;
}

RuMetroAspectState
SignalRuMetroSpeedLimit::aspectState(const std::string& serializedState) const
{
    SignalStateRuMetro state;
    state.mergeSerializedPartialState(serializedState);
    return *state.actualOrRequestedMainAspectState();
}

const SignalSpeedLimit::SpeedLimitForType::Limit*
SignalRuMetroSpeedLimit::limit(RuMetroAspectState state) const
{
    const int routeType = autoblockingRouteType();
    const auto it = m_alsLimits.find(routeType);
    if (it == m_alsLimits.end())
    {
        return nullptr;
    }

    const auto& limits = it->second;
    const auto it1 = limits.find(state);
    if (it1 == limits.end())
    {
        return nullptr;
    }

    return &(it1->second);
}

const SignalRuMetroSpeedLimit::ArsSpeedLimitsMap*
SignalRuMetroSpeedLimit::arsLimitsMap() const
{
    const auto it = m_arsLimits.find(m_routeType);
    if (it != m_arsLimits.end())
    {
        return &(it->second);
    }
    return nullptr;
}

void
SignalRuMetroSpeedLimit::setRouteType(int routeType)
{
    m_routeType = routeType;
}

std::optional<RuMetroAlsCode>
SignalRuMetroSpeedLimit::alsCodeFor(int sections) const
{
    if (sections < 1)
    {
        return {};
    }

    const ArsSpeedLimitsMap* limitsMap = arsLimitsMap();
    if (!limitsMap)
    {
        return {};
    }

    for (auto rit = limitsMap->rbegin(); rit != limitsMap->rend(); rit++)
    {
        const auto& [sects, alsCode] = *rit;
        if (sections >= sects)
            return alsCode;
    }
    return {};
}

std::optional<int>
SignalRuMetroSpeedLimit::speedLimitFor(RuMetroAspectState state) const
{
    RuMetroAspectState limitState =
        state == RuMetroAspectState::GreenMore ? RuMetroAspectState::Green : state;
    auto const* lim = limit(limitState);
    if (lim)
    {
        const double speedMs = lim->speedMs;
        return std::round(UnitConv::ms2kmh(speedMs));
    }
    return {};
}

bool
SignalRuMetroSpeedLimit::noFrequencyFor(RuMetroAspectState state) const
{
    const RuMetroAspectState limitState =
        state == RuMetroAspectState::GreenMore ? RuMetroAspectState::Green : state;
    auto const* lim = limit(limitState);
    if (lim)
    {
        const double speedMs = lim->speedMs;
        return speedMs == -1.0;
    }
    return false;
}

int
SignalRuMetroSpeedLimit::sectionsFor(RuMetroAspectState state, int defaultValue) const
{
    const bool greenMore = state == RuMetroAspectState::GreenMore;
    RuMetroAspectState limitState = greenMore ? RuMetroAspectState::Green : state;
    auto const* lim = limit(limitState);
    if (lim)
    {
        const double speedMs = lim->speedMs;
        auto sections = lim->sections.value_or(defaultValue);
        return greenMore ? sections + 1 : sections;
    }
    return defaultValue;
}
