#pragma once

#include <chrono>
#include <limits>
#include <sstream>

class Clock;
class TimePoint;
class Duration;

namespace ztime
{
    extern Clock clock[128];
    //extern std::array<Clock, 128> clock;
    extern TimePoint Main();
    extern TimePoint Game();
}

enum TimeUnits : int64_t
{
    NANOSECONDS  = 1,
    MICROSECONDS = 1000,
    MILLISECONDS = 1000000,
    SECONDS      = 1000000000,
    MINUTES      = 60000000000,
    HOURS        = 3600000000000
};

class Duration
{
    int64_t _d;
public:
    Duration() : _d(0) {}
    Duration(int64_t duration, TimeUnits u = NANOSECONDS) : _d(duration * u) {}
    Duration(const Duration& d)
    {
        _d = d.GetTicks();
    }
    void operator=(const Duration& d)
    {
        _d = d.GetTicks();
    }
    void operator+=(const Duration& d)
    {
        _d += d.GetTicks();
    }
    void operator-=(const Duration& d)
    {
        _d -= d.GetTicks();
    }
    Duration operator+(const Duration& d)
    {
        return _d + d.GetTicks();
    }
    Duration operator-(const Duration& d)
    {
        return _d - d.GetTicks();
    }

    int64_t GetDuration(TimeUnits u = MICROSECONDS) const
    {
        return _d / u;
    }
    void SetDuration(int64_t duration, TimeUnits u = MICROSECONDS)
    {
        _d = duration * u;
    }
    int64_t GetTicks() const
    {
        return _d;
    }

    static Duration Max()
    {
        return Duration(std::numeric_limits<int64_t>::max(), NANOSECONDS);
    }

    static Duration Min()
    {
        return Duration(std::numeric_limits<int64_t>::min(), NANOSECONDS);
    }
};

class TimePoint
{
    int64_t _t;
public:
    TimePoint() : _t(0) {}
    TimePoint(int64_t time, TimeUnits u = NANOSECONDS) : _t(time * u) {}
    TimePoint(const TimePoint& tp)
    {
        _t = tp.GetTicks();
    }
    void operator=(const TimePoint& tp)
    {
        _t = tp.GetTicks();
    }
    void operator+=(const Duration& d)
    {
        _t += d.GetTicks();
    }
    void operator-=(const Duration& d)
    {
        _t -= d.GetTicks();
    }
    TimePoint operator+(const Duration& d)
    {
        return _t + d.GetTicks();
    }
    TimePoint operator-(const Duration& d)
    {
        return _t - d.GetTicks();
    }

    int64_t GetTime(TimeUnits u = MICROSECONDS) const
    {
        return _t / u;
    }
    void SetTime(int64_t time, TimeUnits u = MICROSECONDS)
    {
        _t = time * u;
    }
    int64_t GetTicks() const
    {
        return _t;
    }

    static TimePoint Max()
    {
        return TimePoint(std::numeric_limits<int64_t>::max(), NANOSECONDS);
    }

    static TimePoint Min()
    {
        return TimePoint(std::numeric_limits<int64_t>::min(), NANOSECONDS);
    }
};

enum class TimeStringFormat
{
    HH_MM_SS,
};

std::string TimeToString(TimePoint time, TimeStringFormat format = TimeStringFormat::HH_MM_SS);
std::string DurationToString(Duration duration, TimeStringFormat format = TimeStringFormat::HH_MM_SS);

class Clock
{
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastSyncTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> _currentSystemTime;
    double _speed = 1.0;
    bool _paused = false;
    TimePoint _currentTime = 0;

public:
    Clock()
    {
        _currentSystemTime = std::chrono::high_resolution_clock::now();
        _lastSyncTime = _currentSystemTime;
    }
    Clock(TimePoint startTime) : Clock()
    {
        _currentTime = startTime;
    }
    Clock(const Clock& c)
    {
        _currentSystemTime = c._currentSystemTime;
        _lastSyncTime = c._lastSyncTime;
        _speed = c._speed;
        _paused = c._paused;
        _currentTime = c._currentTime;
    }
    Clock(Clock&& c) noexcept
    {
        _currentSystemTime = c._currentSystemTime;
        _lastSyncTime = c._lastSyncTime;
        _speed = c._speed;
        _paused = c._paused;
        _currentTime = c._currentTime;
    }
    Clock operator= (const Clock& c)
    {
        _currentSystemTime = c._currentSystemTime;
        _lastSyncTime = c._lastSyncTime;
        _speed = c._speed;
        _paused = c._paused;
        _currentTime = c._currentTime;
        return *this;
    }
    Clock operator= (Clock&& c) noexcept
    {
        _currentSystemTime = c._currentSystemTime;
        _lastSyncTime = c._lastSyncTime;
        _speed = c._speed;
        _paused = c._paused;
        _currentTime = c._currentTime;
        return *this;
    }

    void Update()
    {
        _currentSystemTime = std::chrono::high_resolution_clock::now();
        if (!_paused) {
            _currentTime += (int64_t)((double)std::chrono::duration_cast<std::chrono::nanoseconds>(_currentSystemTime - _lastSyncTime).count() * _speed);
        }
        _lastSyncTime = _currentSystemTime;
    }

    TimePoint Now() const
    {
        return _currentTime;
    }

    void SetSpeed(double speed)
    {
        _speed = speed;
    }

    double GetSpeed() const
    {
        return _speed;
    }

    void IncreaseSpeed(double by)
    {
        _speed += by;
    }

    void DecreaseSpeed(double by)
    {
        _speed -= by;
    }

    void Start()
    {
        _paused = false;
    }

    void Stop()
    {
        _paused = true;
    }

    bool Paused() const
    {
        return _paused;
    }

    void SetTime(TimePoint time)
    {
        _currentTime = time;
    }

    void AdvanceTime(Duration by)
    {
        _currentTime += by;
    }

    void Reset()
    {
        _currentSystemTime = std::chrono::high_resolution_clock::now();
        _lastSyncTime = _currentSystemTime;
        _speed = 1.0;
        _paused = false;
        _currentTime = 0;
    }
};

class SimpleTimer
{
    Clock _internalClock;
public:
    SimpleTimer() : _internalClock(0) {}
    Clock& InternalClock() { return _internalClock; }
    int64_t HoursElapsed() { _internalClock.Update(); return _internalClock.Now().GetTime(HOURS); }
    int64_t MinutesElapsed() { _internalClock.Update(); return _internalClock.Now().GetTime(MINUTES); }
    int64_t SecondsElapsed() { _internalClock.Update(); return _internalClock.Now().GetTime(SECONDS); }
    int64_t MillisElapsed() { _internalClock.Update(); return _internalClock.Now().GetTime(MILLISECONDS); }
    int64_t MicrosElapsed() { _internalClock.Update(); return _internalClock.Now().GetTime(MICROSECONDS); }
    int64_t NanosElapsed() { _internalClock.Update(); return _internalClock.Now().GetTime(NANOSECONDS); }
};

enum ClockType
{
    CLOCK_MAIN,
    CLOCK_GAME
};

inline bool operator< (const Duration& d1, const Duration& d2)
{
    return d1.GetTicks() < d2.GetTicks();
}

inline bool operator> (const Duration& d1, const Duration& d2)
{
    return d1.GetTicks() > d2.GetTicks();
}

inline bool operator<= (const Duration& d1, const Duration& d2)
{
    return d1.GetTicks() <= d2.GetTicks();
}

inline bool operator>= (const Duration& d1, const Duration& d2)
{
    return d1.GetTicks() >= d2.GetTicks();
}

inline bool operator== (const Duration& d1, const Duration& d2)
{
    return d1.GetTicks() == d2.GetTicks();
}

inline bool operator!= (const Duration& d1, const Duration& d2)
{
    return d1.GetTicks() != d2.GetTicks();
}

inline Duration operator- (const TimePoint& t1, const TimePoint& t2)
{
    return t1.GetTicks() - t2.GetTicks();
}

inline bool operator< (const TimePoint& t1, const TimePoint& t2)
{
    return t1.GetTicks() < t2.GetTicks();
}

inline bool operator> (const TimePoint& t1, const TimePoint& t2)
{
    return t1.GetTicks() > t2.GetTicks();
}

inline bool operator<= (const TimePoint& t1, const TimePoint& t2)
{
    return t1.GetTicks() <= t2.GetTicks();
}

inline bool operator>= (const TimePoint& t1, const TimePoint& t2)
{
    return t1.GetTicks() >= t2.GetTicks();
}

inline bool operator== (const TimePoint& t1, const TimePoint& t2)
{
    return t1.GetTicks() == t2.GetTicks();
}