#include "debug_timer.h"

DebugTimer::DebugTimer(const std::string& name, bool showMilliseconds)
    : name(name), show_millis(showMilliseconds),
      start_point(std::chrono::high_resolution_clock::now())
{
}

DebugTimer::~DebugTimer()
{
    if (!stopped)
        stop();
}

long long DebugTimer::stop(bool print)
{
    auto end_point = std::chrono::high_resolution_clock::now();
    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_point).time_since_epoch().count();
    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(end_point).time_since_epoch().count();
    long long duration = end - start;

    if (print)
    {
        std::string output = std::string(name) + ": " + std::to_string(duration) + "us";
        if (show_millis)
        {
            output += " (";
            output += std::to_string((float)duration * 0.001f) + "ms)";
        }
        output += '\n';
        SK_TRACE(output);
    }

    stopped = true;

    return duration;
}