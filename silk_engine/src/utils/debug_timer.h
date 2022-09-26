#pragma once

class DebugTimer
{
public:
    DebugTimer(std::string_view name = "Timer");

    ~DebugTimer();

    //Stops the timer and returns time in milliseconds (or prints it if print parameter is true)
    void operator()();
    void reset();

private:
    double start = 0.0;
    std::string name = "Timer";
    bool is_reset = true;
};