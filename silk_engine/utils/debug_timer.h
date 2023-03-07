#pragma once

class DebugTimer
{
public:
    DebugTimer(std::string_view name = "Timer");

    ~DebugTimer();

    void operator()();
    void reset();

private:
#ifdef SK_ENABLE_DEBUG_OUTPUT
    double start = 0.0;
    std::string name = "Timer";
    bool is_reset = true;
#endif
};