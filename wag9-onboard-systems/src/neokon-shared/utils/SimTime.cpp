#include "SimTime.h"

#include "Scenario.pb.h"

namespace
{
int currentYear()
{
    std::tm localTime{};
    std::time_t currentTime = std::time(nullptr);
    localtime_s(&localTime, &currentTime);

    return 1900 + localTime.tm_year;
}

/**
 * Discards minutes, hours, and seconds from the date-time settings stored in the @p scenario.
 * @ returns the resulting date-time in nanoseconds.
 */
std::chrono::nanoseconds
scenarioDayInNs(proto::Scenario const& scenario)
{
    const SimTm scenarioSimTime{
        convertSimTime(std::chrono::nanoseconds{scenario.date_settings().sim_time_ns()})};

    return convertToSimTime(
        {scenarioSimTime.year, scenarioSimTime.month, scenarioSimTime.day, 0, 0, 0});
}

/**
 * Converts the date saved in the @p scenario to nanoseconds.
 * Used for deprecated scenarios that lack the date_settings().sim_time_ns() field.
 */
std::chrono::nanoseconds
legacyScenarioDayInNs(proto::Scenario const& scenario)
{
    return convertToSimTime(
        {currentYear(), 1, static_cast<int>(scenario.date_settings().day_of_year()) + 1, 0, 0, 0});
}

/**
 * Converts @p dateSettings to nanoseconds.
 * Used for deprecated scenarios that lack the date_settings().sim_time_ns() field.
 */
std::chrono::nanoseconds
legacyCalcTimePointFromDateSettings(const proto::DateSettings& dateSettings, int hours, int minutes,
    int seconds)
{
    const SimTm simTm
    {
        currentYear(),
        1,
        static_cast<int>(dateSettings.day_of_year()) + 1,
        hours,
        minutes,
        0,
    };
    return convertToSimTime(simTm) +
        std::chrono::duration<int>(dateSettings.time_of_day_offset_sec());
}
} // anonymous namespace

/**
 * Convert nanoseconds since 1970 year to readable date
 */
SimTm
convertSimTime(std::chrono::nanoseconds simTime)
{
    using namespace std;
    using namespace chrono;

    time_t tt = duration_cast<seconds>(simTime).count();
    tm utcTm{};
    gmtime_s(&utcTm, &tt);

    SimTm res;

    res.year = utcTm.tm_year + 1900;
    res.month = utcTm.tm_mon + 1;
    res.day = utcTm.tm_mday;
    res.hour = utcTm.tm_hour;
    res.min = utcTm.tm_min;
    res.second = utcTm.tm_sec;

    return res;
}

double
getSecondsInDay(std::chrono::nanoseconds simTime)
{
    using namespace std;
    using namespace chrono;

    using days = duration<int, ratio_multiply<hours::period, ratio<24>>::type>;
    nanoseconds secondsInDay = simTime;
    days d = duration_cast<days>(secondsInDay);
    secondsInDay -= d;

    return duration_cast<duration<double>>(secondsInDay).count();
}

std::chrono::nanoseconds
getNanosecondsInDay(std::chrono::nanoseconds simTime)
{
    using namespace std;
    using namespace chrono;

    using days = duration<int, ratio_multiply<hours::period, ratio<24>>::type>;
    nanoseconds nanosecondsInDay = simTime % days(1);

    return nanosecondsInDay;
}

std::chrono::nanoseconds
convertToSimTime(SimTm const& simTm)
{
    using namespace std;
    using namespace chrono;
    tm utcTm{};

    utcTm.tm_sec = simTm.second;
    utcTm.tm_min = simTm.min;
    utcTm.tm_hour = simTm.hour;
    utcTm.tm_mday = simTm.day;
    utcTm.tm_mon = simTm.month - 1;
    utcTm.tm_year = simTm.year - 1900;
    utcTm.tm_isdst = 0;

    // mktime returns time as if we set it in local calendar time, but @p convertSimTime function
    // uses global time. So we convert the result to UTC time by subtracting the current
    // timezone time.
    _tzset();
    time_t res = std::mktime(&utcTm);
    long timezoneOffset;
    _get_timezone(&timezoneOffset);

    return duration_cast<nanoseconds>(seconds(res - timezoneOffset));
}

std::chrono::nanoseconds
calcStartDayTimeFromScenario(proto::Scenario const& scenario)
{
    return scenario.date_settings().has_sim_time_ns() ?
        scenarioDayInNs(scenario) : legacyScenarioDayInNs(scenario);
}

std::chrono::nanoseconds
calcTimePointFromScenario(proto::Scenario const& scenario)
{
    return scenario.date_settings().has_sim_time_ns()
        ? std::chrono::nanoseconds{scenario.date_settings().sim_time_ns()}
        : legacyCalcTimePointFromDateSettings(
              scenario.date_settings(),
              scenario.time_of_day_hh(),
              scenario.time_of_day_mm(),
              0);
}

std::chrono::nanoseconds calcTimePointFromDateSettings(proto::DateSettings const& dateSettings,
    int hour, int minute, int second)
{
    return dateSettings.has_sim_time_ns()
        ? std::chrono::nanoseconds{dateSettings.sim_time_ns()}
        : legacyCalcTimePointFromDateSettings(dateSettings, hour, minute, second);
}

void
splitDateAndTime(
    std::chrono::nanoseconds simTime,
    std::chrono::nanoseconds& date,
    std::chrono::nanoseconds& time)
{
    // TODO std::chrono::days
    std::chrono::nanoseconds timePart = simTime % std::chrono::hours(24);
    date = simTime - timePart;
    time = simTime - timePart;
}
