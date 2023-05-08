#include "application.h"
#include "time.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "core/input/input.h"
#include "scene/scene.h"
#include "gfx/renderer.h"
#include "gfx/devices/logical_device.h"
#include "scene/camera/camera.h"

Application::Application()
{
}

void Application::run()
{
    while (running)
    {
        if (Window::getActive().isMinimized())
            glfwWaitEvents();
        else
        {
            Input::update();
        }
            
        update();
    }
    RenderContext::getLogicalDevice().wait();
}

void Application::update()
{
    if (!running || Window::getActive().isMinimized())
        return;

    Renderer::wait();

    onUpdate();
    if (Scene::getActive())
        Scene::getActive()->onUpdate();

    Renderer::render();

    Time::update();
}