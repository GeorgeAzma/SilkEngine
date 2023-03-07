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
        double duration = Time::getHighResTime() - start;
        if (name.size())
            SK_TRACE("{}: {:.3g}", name, std::Seconds(duration));
        else
            SK_TRACE("{:.3g}", std::Seconds(duration));
        start = Time::getHighResTime();
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
    void DebugTimer::reset() {}
#endif