#include "application.h"
#include "time.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "scene/scene.h"
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

        while (Window::getActive().isMinimized() && running)
            glfwWaitEvents();

        if (!running)
            break;

        update();
    }
    RenderContext::getLogicalDevice().wait();
}

void Application::update()
{
    Renderer::wait();

    onUpdate();
    Scene::updateScenes();

    Renderer::render(Scene::getActive().get() ? Scene::getActive()->getMainCamera() : nullptr);

    Window::getActive().update();
    Time::update();
}