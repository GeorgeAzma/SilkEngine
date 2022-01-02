#pragma once

#include <functional>
#include "delta.h"

class FixedUpdate
{
public:
    using UpdateFunc = std::function<void(double)>;
    FixedUpdate(unsigned int maximumFPS, UpdateFunc func = nullptr);
    bool update();
    unsigned int getFPS() const { return round(fps); }
    double getFPSf() const { return fps; }
    double getElapsedTime() const { return elapsed; }
    double getRuntime() const { return totalTime; }
    unsigned int getFramesPassed() const { return frames; }
    unsigned int getMaxFPS() const { return maxFPS; }
    double getTimestep() const { return timestep; }
    void setMaxFPS(unsigned int newMaxFPS)
    {
        maxFPS = newMaxFPS;
        timestep = 1.0 / newMaxFPS;
    }
    void printFPS();

private:
    Delta<double> delta;
    UpdateFunc updateFunction;
    unsigned int maxFPS;
    double elapsed;
    double fps;
    unsigned int frames;
    double timestep;
    double totalTime;
};