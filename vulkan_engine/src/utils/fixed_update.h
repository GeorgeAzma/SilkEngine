#pragma once
#include <functional>
#include "delta.h"

class FixedUpdate
{
public:
    using UpdateFunc = std::function<void(float)>;
    FixedUpdate(unsigned int maximumFPS, UpdateFunc func = nullptr);
    bool update();
    inline unsigned int getFPS() const { return (unsigned int)round(fps); }
    inline float getFPSf() const { return fps; }
    inline float getElapsedTime() const { return elapsed; }
    inline float getRuntime() const { return totalTime; }
    inline unsigned int getFramesPassed() const { return frames; }
    inline unsigned int getMaxFPS() const { return maxFPS; }
    inline float getTimestep() const { return timestep; }
    inline void setMaxFPS(const unsigned int newMaxFPS)
    {
        maxFPS = newMaxFPS;
        timestep = 1.0f / newMaxFPS;
    }
    void printFPS();

private:
    Delta<float> delta;
    UpdateFunc updateFunction;
    unsigned int maxFPS;
    float elapsed;
    float fps;
    unsigned int frames;
    float timestep;
    float totalTime;
};