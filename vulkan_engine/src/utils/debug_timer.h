#pragma once

#include <chrono>

class DebugTimer
{
public:
    DebugTimer(const char *name = "Timer", bool showMilliseconds = true);

    ~DebugTimer();

    //Stops the timer and returns time in microseconds (or prints it if print parameter is true)
    long long stop(bool print = true);

private:
    std::chrono::time_point<std::chrono::steady_clock> start_point;
    const char *name = "Timer";
    bool stopped = false;
    bool show_millis = true;
};