#include "debug_timer.h"

#ifdef SK_ENABLE_DEBUG_OUTPUT
    DebugTimer::DebugTimer(std::string_view name)
        : name(name), start(Time::getHighResTime())
    {
    }
    
    DebugTimer::~DebugTimer()
    {
        if (is_reset) operator()();
    }
    
    void DebugTimer::operator()()
    {
        is_reset = false;
        double delta = Time::getHighResTime() - start;
        if (name.size())
            SK_TRACE("{}: {:.3g}", name, std::Seconds(delta));
        else
            SK_TRACE("{:.3g}", std::Seconds(delta));
        start = Time::getHighResTime();
    }

    void DebugTimer::sample(size_t max_samples)
    {
        is_reset = false;
        double delta = Time::getHighResTime() - start;
        average += delta;
        ++samples;
        start = Time::getHighResTime();
        if (samples >= max_samples)
        {
            SK_TRACE("{}: {:.3g}", name, std::Seconds(average / samples));
            average = 0.0;
            samples = 0;
        }
    }
    
    void DebugTimer::reset()
    {
        start = Time::getHighResTime();
        is_reset = true;
    }
#else
    DebugTimer::DebugTimer(std::string_view name) {}
    DebugTimer::~DebugTimer() {}    
    void DebugTimer::operator()() {}   
    void DebugTimer::sample(size_t max_samples) {}
    void DebugTimer::reset() {}
#endif