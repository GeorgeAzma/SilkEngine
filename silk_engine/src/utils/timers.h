#pragma once

#include "core/time.h"

class Timers
{
    class Timer
    {
        friend class Timers;

    public:
        template <class Rep, class Period>
        Timer(const std::chrono::duration<Rep, Period>& interval, int repeat, std::function<void()>&& on_tick)
            : repeat(repeat),
            interval(std::chrono::duration_cast<std::chrono::nanoseconds>(interval).count() * 0.000000001),
            next(Time::getTime() + this->interval),
            on_tick(on_tick)
        {
        }

    private:
        double interval;
        double next;
        int repeat;
        std::function<void()> on_tick;
    };

    class FrameTimer
    {
        friend class Timers;

    public:
        FrameTimer(unsigned int frame_interval, int repeat, std::function<void()>&& on_tick);

        operator unsigned int() const { return next_frame; }

    private:
        unsigned int frame_interval;
        unsigned int next_frame;
        int repeat;
        std::function<void()> on_tick;
    };

public:
    template <class Rep, class Period>
    static void once(const std::chrono::duration<Rep, Period>& delay, std::function<void()>&& function)
    {
        timers.emplace_back(delay, 1, std::forward<std::function<void()>>(function));
    }

    template <class Rep, class Period>
    static void every(const std::chrono::duration<Rep, Period>& interval, std::function<void()>&& function)
    {
        timers.emplace_back(interval, -1, std::forward<std::function<void()>>(function));
    }

    template <class Rep, class Period>
    static void repeat(const std::chrono::duration<Rep, Period>& interval, uint32_t repeat, std::function<void()>&& function)
    {
        timers.emplace_back(interval, repeat, std::forward<std::function<void()>>(function));
    }

    static void frameOnce(unsigned int frame_delay, std::function<void()>&& function);
    static void frameEvery(unsigned int frame_interval, std::function<void()>&& function);
    static void frameRepeat(unsigned int frame_interval, uint32_t repeat, std::function<void()>&& function);
    static void resetFrameTimers();  
    static void resetTimers();
    static void reset();

    static void update();

private:
    static inline std::vector<Timer> timers;
    static inline std::vector<FrameTimer> frame_timers;
};
