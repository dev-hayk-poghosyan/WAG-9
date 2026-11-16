#include "SignalSpeedLimit.h"

SignalSpeedLimit::SignalSpeedLimit()
{
}

SignalSpeedLimit::SignalSpeedLimit(std::vector<SpeedLimitForType>&& limits)
    : m_speedLimits(std::move(limits))
{
}

std::vector<SignalSpeedLimit::SpeedLimitForType> const&
SignalSpeedLimit::speedLimits() const
{
    return m_speedLimits;
}
