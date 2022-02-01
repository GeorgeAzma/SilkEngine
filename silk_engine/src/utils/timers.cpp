#include "timers.h"

void Timers::frameOnce(unsigned int frame_delay, std::function<void()>&& function)
{
    frame_timers.emplace_back(frame_delay, 1, std::forward<std::function<void()>>(function));
}

void Timers::frameEvery(unsigned int frame_interval, std::function<void()>&& function)
{
    frame_timers.emplace_back(frame_interval, -1, std::forward<std::function<void()>>(function));
}

void Timers::frameRepeat(unsigned int frame_interval, uint32_t repeat, std::function<void()>&& function)
{
    frame_timers.emplace_back(frame_interval, repeat, std::forward<std::function<void()>>(function));
}

void Timers::resetFrameTimers()
{
    frame_timers.clear();
}

void Timers::resetTimers()
{
    timers.clear();
}

void Timers::reset()
{
    resetTimers();
    resetFrameTimers();
}

void Timers::update()
{
    for (size_t i = 0; i < frame_timers.size(); ++i)
    {
        auto& instance = frame_timers[i];
        if (Time::frame >= instance)
        {
            instance.on_tick();

            instance.next_frame += instance.frame_interval;

            if (--instance.repeat == 0)
            {
                std::swap(instance, frame_timers.back());
                frame_timers.pop_back();
            }
        }
    }

    for (size_t i = 0; i < timers.size(); ++i)
    {
        auto& instance = timers[i];

        if (Time::getTime() >= instance.next)
        {
            instance.on_tick();

            instance.next += instance.interval;

            if (--instance.repeat == 0)
            {
                std::swap(instance, timers.back());
                timers.pop_back();
            }
        }
    }
}

Timers::FrameTimer::FrameTimer(unsigned int frame_interval, int repeat, std::function<void()>&& on_tick)
    : frame_interval(frame_interval),
    next_frame(Time::frame + frame_interval),
    repeat(repeat),
    on_tick(on_tick)
{
}
