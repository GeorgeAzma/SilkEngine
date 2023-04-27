#include "application.h"
#include "time.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "scene/scene.h"
#include "gfx/renderer.h"
#include "gfx/devices/logical_device.h"
#include "scene/camera/camera.h"

/** TODOs: 
* Subpass attachment references / descriptions.Not all previous subpass outputs are inputs of the next subpass etc.
* Bind vertex/index buffers before actually drawing
*/

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
            glfwPollEvents();
            
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
    Scene::updateScenes();

    Renderer::render();

    Window::getActive().update();
    Time::update();
}