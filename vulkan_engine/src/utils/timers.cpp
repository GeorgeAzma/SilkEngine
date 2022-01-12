#include "timers.h"

void Timers::update()
{
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