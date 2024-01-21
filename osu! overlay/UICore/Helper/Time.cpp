#include "Time.h"

Clock ztime::clock[128] = { TimePoint() };
//std::array<Clock, 128> ztime::clock = std::array<Clock, 128>();

TimePoint ztime::Main()
{
    return clock[CLOCK_MAIN].Now();
}

TimePoint ztime::Game()
{
    return clock[CLOCK_GAME].Now();
}

std::string TimeToString(TimePoint time, TimeStringFormat format)
{
    std::stringstream timeStr;
    int h = time.GetTime(HOURS);
    int m = time.GetTime(MINUTES) % 60;
    int s = time.GetTime(SECONDS) % 60;
    if (h > 0) timeStr << h << ":";
    if (m < 10) timeStr << "0" << m << ":";
    else timeStr << m << ":";
    if (s < 10) timeStr << "0" << s;
    else timeStr << s;
    return timeStr.str();
}

std::string DurationToString(Duration duration, TimeStringFormat format)
{
    return TimeToString(duration.GetTicks(), format);
}