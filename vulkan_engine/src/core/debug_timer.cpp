#include "debug_timer.h"

DebugTimer::DebugTimer(const char *name, bool showMilliseconds)
    : name{name}, show_millis(showMilliseconds),
      start_point(std::chrono::high_resolution_clock::now())
{
}

DebugTimer::~DebugTimer()
{
    if (!stopped)
        stop();
}

void DebugTimer::stop()
{
    auto end_point = std::chrono::high_resolution_clock::now();
    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_point).time_since_epoch().count();
    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_point).time_since_epoch().count();
    long long duration = end - start;
    std::cout << name << ": " << duration << "us";
    if (show_millis)
    {
        const float ms = (float)duration * 0.001f;
        std::cout << " (" << ms << "ms)";
    }
    std::cout << '\n';
    stopped = true;
}