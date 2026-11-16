#pragma once

#include "DateSettings.pb.h"
#include "SunPositionParams.pb.h"
#include "TimeOfDay.h"

#include <optional>

class SunPositionParams
{
public:
    SunPositionParams() = default;

    SunPositionParams(proto::SunPositionParams const& protoParams);

    proto::SunPositionParams toProto() const;

    void setDateSettings(proto::DateSettings const& dateSettings) {
        m_dateSettings = dateSettings;
    }

    /**
     * When forced time of day is used, the sun doesn't move at all.
     */
    void setForcedTimeOfDay(std::optional<TimeOfDay> timeOfDay) {
        m_forcedTimeOfDay = timeOfDay;
    }

    /**
     * If a forced time of day is set, returns some fixed day of year. Otherwise, takes
     * it from date settings.
     */
    int dayOfYear() const;

    /**
     * If a forced time of day is set, returns a fixed number of seconds corresponding
     * to either midnight or noon. Otherwise, takes the argument and adjusts it according
     * to date settings.
     */
    float adjustedSecondsSinceMidnight(float secondsSinceMidnignt) const;

private:
    proto::DateSettings m_dateSettings;
    std::optional<TimeOfDay> m_forcedTimeOfDay;
};
