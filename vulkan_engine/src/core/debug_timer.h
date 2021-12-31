#pragma once
#include <chrono>

class DebugTimer
{
public:
    DebugTimer(const char *name = "Timer", bool showMilliseconds = true);

    ~DebugTimer();

    void stop();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_point;
    const char *name = "Timer";
    bool stopped = false;
    bool show_millis = true;
};