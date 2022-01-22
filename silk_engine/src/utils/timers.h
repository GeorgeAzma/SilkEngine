#pragma once

#include "core/time.h"

class Timers
{
    class Timer
    {
        friend class Timers;

    public:
        template <typename T>
        Timer(const std::chrono::duration<T>& interval, int repeat, std::function<void()>&& on_tick)
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
        template <typename T>
        FrameTimer(unsigned int frame_interval, int repeat, std::function<void()>&& on_tick)
            : frame_interval(frame_interval),
            next_frame(Time::frames + frame_interval), 
            repeat(repeat), 
            on_tick(on_tick)
        {
        }

        operator unsigned int() const { return next_frame; }

    private:
        unsigned int frame_interval;
        unsigned int next_frame;
        int repeat;
        std::function<void()> on_tick;
    };

public:
    template <typename T>
    static void once(const std::chrono::duration<T>& delay, std::function<void()>&& function)
    {
        timers.emplace_back(delay, 1, std::forward<std::function<void()>>(function));
    }

    template <typename T>
    static void every(const std::chrono::duration<T>& interval, std::function<void()>&& function)
    {
        timers.emplace_back(interval, -1, std::forward<std::function<void()>>(function));
    }

    template <typename T>
    static void repeat(const std::chrono::duration<T>& interval, uint32_t repeat, std::function<void()>&& function)
    {
        timers.emplace_back(interval, repeat, std::forward<std::function<void()>>(function));
    }

    template <typename T>
    static void once(unsigned int frame_delay, std::function<void()>&& function)
    {
        frame_timers.emplace_back(frame_delay, 1, std::forward<std::function<void()>>(function));
    }

    template <typename T>
    static void every(unsigned int frame_interval, std::function<void()>&& function)
    {
        frame_timers.emplace_back(frame_interval, -1, std::forward<std::function<void()>>(function));
    }

    template <typename T>
    static void repeat(unsigned int frame_interval, uint32_t repeat, std::function<void()>&& function)
    {
        frame_timers.emplace_back(frame_interval, repeat, std::forward<std::function<void()>>(function));
    }

    static void resetFrameTimers()
    {
        frame_timers.clear();
    }
    
    static void resetTimers()
    {
        timers.clear();
    }

    static void reset()
    {
        resetTimers();
        resetFrameTimers();
    }

    static void update();

private:
    static inline std::vector<Timer> timers;
    static inline std::vector<FrameTimer> frame_timers;
};
