#include "application.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/core/input/input.h"
#include "silk_engine/scene/scene.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/scene/camera/camera.h"

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
            Input::update();
            
        update();
    }
    RenderContext::getLogicalDevice().wait();
}

void Application::update()
{
    if (!running || Window::getActive().isMinimized())
        return;

    onUpdate();
    Time::update();
}