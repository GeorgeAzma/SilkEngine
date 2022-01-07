#pragma once

#include <functional>
#include "delta.h"

class FixedUpdate
{
public:
    FixedUpdate(double interval);
    bool update();
    unsigned int getFPS() const { return round(fps); }
    double getFPSf() const { return fps; }
    double getDeltaTime() const { return delta; }
    double getRuntime() const { return total_time; }
    unsigned int getFramesPassed() const { return frames; }
    unsigned int getMaxFPS() const { return 1.0 / interval; }
    double getInterval() const { return interval; }
    void setInterval(double interval)
    {
        interval = interval;
    }

private:
    Delta<double> delta;
    double elapsed = 0;
    double fps = 0;
    unsigned int frames = 0;
    double interval = 0;
    double total_time = 0;
};