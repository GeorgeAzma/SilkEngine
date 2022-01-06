#include "fixed_update.h"
#include "core/time.h"

FixedUpdate::FixedUpdate(double interval)
    : interval(interval), delta(Time::getSystemTime()) {}

bool FixedUpdate::update()
{
    double deltaTime = delta(Time::getSystemTime());
    elapsed += deltaTime;
    total_time += deltaTime;

    if (elapsed >= interval)
    {
        if (elapsed)
            fps = 1.0 / elapsed;
        else 
            fps = std::numeric_limits<decltype(fps)>::max();

        ++frames;
        elapsed = 0;
        return true;
    }
    return false;
}