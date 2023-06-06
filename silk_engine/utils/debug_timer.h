#pragma once

class DebugTimer
{
public:
#ifdef SK_ENABLE_DEBUG_OUTPUT
    DebugTimer(std::string_view name)
        : name(name), start(Time::getHighResTime()) {}

    void begin()
    {
        if (isStopped())
            return;
        std::scoped_lock lock(mux);
        start = Time::getHighResTime();
    }

    void end()
    {
        std::scoped_lock lock(mux);
        elapsed += Time::getHighResTime() - start;
        ++samples;
    }

    void reset()
    {
        std::scoped_lock lock(mux);
        elapsed = 0.0;
        samples = 0;
    }

    void stop()
    {
        std::scoped_lock lock(mux);
        start = -1.0;
    }

    void print(double elapsed) const
    {
        if (name.size())
            SK_TRACE("{}: {:.3g}", name, std::Seconds(elapsed));
        else
            SK_TRACE("{:.3g}", std::Seconds(elapsed));
    }

    void print()
    {
        if (elapsed <= 0.0)
            end();
        print(elapsed);
    }

    double getElapsed() const { return elapsed; }
    size_t getSamples() const { return samples; }
    double getAverage() const { return elapsed / samples; }
    bool isStopped() const { return start < 0.0; }

private:
    double start = 0.0;
    double elapsed = 0.0;
    size_t samples = 0;
    std::string name = "Timer";
    std::mutex mux;
#else
    void begin() {}
    void end() {}
    void reset() {}
    void stop() {}
    void print(double elapsed) const {}
    void print() {}
    double getElapsed() const { return 0.0; }
    size_t getSamples() const { return 0; }
    double getAverage() const { return 0.0; }
    bool isStopped() const { return false; }
#endif
};