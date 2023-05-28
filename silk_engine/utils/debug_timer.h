#pragma once

class DebugTimer
{
public:
    DebugTimer(std::string_view name = "");

    ~DebugTimer();

    void operator()(); 
    void sample(size_t max_samples = 32);
    void reset();

private:
#ifdef SK_ENABLE_DEBUG_OUTPUT
    double start = 0.0;
    double average = 0.0;
    size_t samples = 0;
    std::string name = "Timer";
    bool is_reset = true;
#endif
};