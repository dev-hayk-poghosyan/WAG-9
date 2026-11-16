#pragma once

#include <chrono>

namespace proto
{
class Scenario;
class DateSettings;
}

/**
 * Structure to represent time in simulation.
 */
struct SimTm
{
    int year = 1970; // 1970-2400
    int month = 1;   // 1-12
    int day = 1;     // 1-31
    int hour = 0;    // 0-23
    int min = 0;     // 0-59
    int second = 0;  // 0-59
};

/**
* Convert simTime to readable representation
*/
SimTm convertSimTime(std::chrono::nanoseconds simTime);

/**
 * Convert simTime to seconds in current day since 0:00
 */
double getSecondsInDay(std::chrono::nanoseconds simTime);

/**
 * Convert simTime to nanoseconds in current day since 0:00
 */
std::chrono::nanoseconds getNanosecondsInDay(std::chrono::nanoseconds simTime);

/**
 * Convert SimTm to simTime nanoseconds
 */
std::chrono::nanoseconds convertToSimTime(SimTm const& tm);

/**
 * Calc day time start time point from scenario specified date
 */
std::chrono::nanoseconds calcStartDayTimeFromScenario(proto::Scenario const& scenario);

/**
 * Calc time point from the date and time of day specified by the scenario.
 */
std::chrono::nanoseconds calcTimePointFromScenario(proto::Scenario const& scenario);

/**
 * Calculates nanoseconds from date settings and time of day.
 * @p dateSettings may contain full date-time in ns (optional), or only year, month and day. 
 * @p hour, @p minute and @p second are time of day.
 * If dateSettings contains full date-time, then @p hour, @p minute and @p second are ignored.
 */
std::chrono::nanoseconds calcTimePointFromDateSettings(proto::DateSettings const& dateSettings,
    int hour, int minute, int second);

/**
 * Split current day time(HH:MM:SS+ns) and date(DD.MM.YYYY)
 * time = simTime % day
 * date = simTime - time
 */
void splitDateAndTime(
    std::chrono::nanoseconds simTime,
    std::chrono::nanoseconds& date,
    std::chrono::nanoseconds& time);
