#pragma once

#include "neokon-shared/signalling/SignalSpeedLimit.h"

#include "neokon-shared/signalling/ru-metro/RuMetroAspectState.h"
#include "neokon-shared/signalling/RuMetroAlsCode.h"

#include <map>
#include <unordered_map>

/**
 * Used to store and use Metro Limits data.
 */
class SignalRuMetroSpeedLimit
{
public:
    SignalRuMetroSpeedLimit(std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits);

    /**
     * Autoblock mode only.
     * @returns speed limit (km/h) for specific Signal state
     * or std::nullopt if data doesn't exist.
     */
    std::optional<int> speedLimitFor(RuMetroAspectState state) const;

    /**
     * ARS mode only.
     * @returns TRUE on using ALS-ARS.
     */
    bool alsArs() const { return m_alsArs; }

    /**
     * Autoblock mode only.
     * @returns if the signal light sends No Frequency for @p state.
     */
    bool noFrequencyFor(RuMetroAspectState state) const;

    /**
     * Autoblock mode only.
     * @returns free sections number to use specific Signal state
     * or default value if data doesn't exist.
     */
    int sectionsFor(RuMetroAspectState state, int defaultValue) const;

    /**
     * ARS mode only.
     * @returns ALS code for ARS mode
     * or default value if data doesn't exist.
     */
    std::optional<RuMetroAlsCode> alsCodeFor(int sections) const;

    /**
     * Sets current routeType value
     */
    void setRouteType(int routeType);

private:
    typedef SignalSpeedLimit::SpeedLimitForType::Limit SpeedLimit; 
    typedef std::unordered_map<RuMetroAspectState, SpeedLimit> SpeedLimitsMap;
    typedef std::map<int, RuMetroAlsCode> ArsSpeedLimitsMap;

    bool arsMode(std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits) const;
    int  autoblockingRouteType() const;
    void fillAutoblockLimits(std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits);
    void fillArsLimits(std::vector<SignalSpeedLimit::SpeedLimitForType> const& speedLimits);
    const SignalSpeedLimit::SpeedLimitForType::Limit* limit(RuMetroAspectState state) const;
    const ArsSpeedLimitsMap* arsLimitsMap() const;
    RuMetroAspectState aspectState(const std::string& serializedState) const;

    const int defaultAutoblockingRouteType = 0;
    const int defaultArsRouteType = 1;

    const bool m_arsMode;
    int m_routeType = 1;
    bool m_alsArs = false;
    std::unordered_map<int, SpeedLimitsMap> m_alsLimits;
    std::unordered_map<int, ArsSpeedLimitsMap> m_arsLimits;
};
