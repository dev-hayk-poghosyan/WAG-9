#include "SunPositionParams.h"

#include <cmath>

SunPositionParams::SunPositionParams(proto::SunPositionParams const& protoParams)
    : m_dateSettings(protoParams.date_settings())
{
    switch (protoParams.forced_time_of_day())
    {
    case protoParams.FORCED_TIME_OF_DAY_DAY:
        m_forcedTimeOfDay = TimeOfDay::Day;
        break;
    case protoParams.FORCED_TIME_OF_DAY_NIGHT:
        m_forcedTimeOfDay = TimeOfDay::Night;
        break;
    default:
        break;
    }
}

proto::SunPositionParams
SunPositionParams::toProto() const
{
    proto::SunPositionParams protoParams;
    *protoParams.mutable_date_settings() = m_dateSettings;

    if (m_forcedTimeOfDay == TimeOfDay::Day)
    {
        protoParams.set_forced_time_of_day(protoParams.FORCED_TIME_OF_DAY_DAY);
    }
    else if (m_forcedTimeOfDay == TimeOfDay::Night)
    {
        protoParams.set_forced_time_of_day(protoParams.FORCED_TIME_OF_DAY_NIGHT);
    }
    else
    {
        protoParams.set_forced_time_of_day(protoParams.FORCED_TIME_OF_DAY_NONE);
    }

    return protoParams;
}

int
SunPositionParams::dayOfYear() const
{
    if (m_forcedTimeOfDay)
    {
        return 82; // 22-23 of March
    }
    else
    {
        return m_dateSettings.day_of_year();
    }
}

float
SunPositionParams::adjustedSecondsSinceMidnight(float secondsSinceMidnight) const
{
    if (m_forcedTimeOfDay)
    {
        return *m_forcedTimeOfDay == TimeOfDay::Day ? 12.f * 60.f * 60.f : 0.f;
    }
    else
    {
        return std::fmod(secondsSinceMidnight, 24.f * 60.f * 60.f);
    }
}
