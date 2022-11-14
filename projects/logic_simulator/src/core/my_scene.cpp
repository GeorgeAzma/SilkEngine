#include "my_scene.h"
#include "gfx/window/window.h"
#include "simulation/simulation.h"
#include "utils/cooldown.h"

Cooldown c(200ms);

void MyScene::onStart()
{
    Window::getActive().setSize({ 1280, 720 });
    Simulation::init();
}

void MyScene::onUpdate()
{
    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {0} FPS ({1:.4} ms) | {2}x{3}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));
    Simulation::update();
}

void MyScene::onStop()
{
    Simulation::destroy();
}
