#include "debug_timer.h"

DebugTimer::DebugTimer(std::string_view name)
    : name(name), start(Time::runtime)
{
}

DebugTimer::~DebugTimer()
{
    if (is_reset) operator()();
}

void DebugTimer::operator()()
{
    is_reset = false;
    double duration = Time::runtime - start;
    if (name.size())
        SK_TRACE("{}: {:.3g}", name, std::Seconds(duration));
    else
        SK_TRACE("{:.3g}", std::Seconds(duration));
    start = Time::runtime;
}

void DebugTimer::reset()
{
    start = Time::runtime;
    is_reset = true;
}
