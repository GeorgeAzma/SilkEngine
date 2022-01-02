#include "fixed_update.h"

FixedUpdate::FixedUpdate(unsigned int maximumFPS, UpdateFunc func)
    : maxFPS(maximumFPS), updateFunction(func), elapsed(0), fps(0), frames(0), timestep(1.0f / maxFPS), totalTime(0), delta(glfwGetTime()) {}

bool FixedUpdate::update()
{
    const double deltaTime = delta.calc(glfwGetTime());
    elapsed += deltaTime;
    totalTime += deltaTime;
    if (elapsed >= timestep)
    {
        ++frames;
        if (updateFunction)
            updateFunction(elapsed);
        fps = 1.0 / elapsed;
        elapsed -= timestep;
        return true;
    }
    return false;
}

void FixedUpdate::printFPS()
{
    std::cout << "FPS: " << getFPS() << " (" << std::floorf(elapsed * 100000.0f) / 100.0f << " ms)" << std::endl;
}
