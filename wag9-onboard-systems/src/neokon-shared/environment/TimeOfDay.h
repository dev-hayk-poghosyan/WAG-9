#pragma once

#include "TimeOfDay.pb.h"

#include <array>

enum class TimeOfDay
{
    Day = proto::TimeOfDay::DAY,
    Night = proto::TimeOfDay::NIGHT
};

constexpr auto allTimesOfDay()
{
    return std::array{ TimeOfDay::Day, TimeOfDay::Night };
}

inline TimeOfDay
timeOfDayFromProto(proto::TimeOfDay::DayTime dayTime)
{
    return static_cast<TimeOfDay>(dayTime);
}

inline proto::TimeOfDay::DayTime
timeOfDayToProto(TimeOfDay dayTime)
{
    return static_cast<proto::TimeOfDay::DayTime>(dayTime);
}
