#pragma once

#include <optional>
#include <string>
#include <unordered_map>

/**
 * Storage for limits data from Signal property.
 */
class SignalSpeedLimit
{
public:
    struct SpeedLimitForType
    {
        /**
         * Type of limits set.
         */
        int type{};

        /**
         * Data from Signal property.
         */
        struct Limit
        {
            /**
             * Speed limit (m/s).
             */
            double speedMs{};
            /**
             * Free sections ahead to use signal state.
             */
            std::optional<int> sections;
        };

        /**
         * Storage limits per signal type:
         * key - serialized signal partial state,
         * value - Limit.
         */
        std::unordered_map<std::string, Limit> speedByState;
    };

    SignalSpeedLimit();
    SignalSpeedLimit(std::vector<SpeedLimitForType>&& limits);

    std::vector<SpeedLimitForType> const& speedLimits() const;

protected:
    std::vector<SpeedLimitForType> m_speedLimits;
};
