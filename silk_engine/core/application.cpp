#include "application.h"
#include "time.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "scene/scene_manager.h"
#include "gfx/renderer.h"
#include "gfx/devices/logical_device.h"
#include "scene/camera/camera.h"

//TODO: Subpass attachment references/descriptions. Not all previous subpass outputs are inputs of the next subpass etc.
//FIXME: App slowing down every time when window is resized and update is called right after
//FIXME/TODO: Command queue

void Application::run()
{
    while (running)
    {
        glfwPollEvents();

        while (Window::getActive().isMinimized())
            glfwWaitEvents();

        update();
    }
    RenderContext::getLogicalDevice().wait();
}

void Application::update()
{
    Renderer::wait();

    onUpdate();
    SceneManager::update();

    Renderer::render(SceneManager::getActive().get() ? SceneManager::getActive()->getMainCamera() : nullptr);

    Window::getActive().update();
    Time::update();
}