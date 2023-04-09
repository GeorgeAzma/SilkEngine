#include "application.h"
#include "input/input.h"
#include "time.h"
#include "input/keys.h"
#include "gfx/render_context.h"
#include "gfx/devices/physical_device.h"
#include "scene/resources.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "scene/scene_manager.h"
#include "gfx/renderer.h"
#include "gfx/descriptors/descriptor_allocator.h"
#include "gfx/devices/logical_device.h"
#include "scene/camera/camera.h"
#include "gfx/window/glfw.h"

//TODO: Subpass attachment references/descriptions. Not all previous subpass outputs are inputs of the next subpass etc.
//FIXME: App slowing down every time when window is resized and update is called right after
//FIXME/TODO: Command queue

Application::Application(ApplicationCommandLineArgs args)
    : command_line_args(args)
{
    window = new Window();
    Renderer::init();
    Dispatcher::subscribe(*this, &Application::onWindowClose);
    Dispatcher::subscribe(*this, &Application::onFramebufferResize);
    Dispatcher::subscribe(*this, &Application::onKeyPress);
}

Application::~Application()
{
    Dispatcher::unsubscribe(*this, &Application::onWindowClose);
    Dispatcher::unsubscribe(*this, &Application::onFramebufferResize);
    Dispatcher::unsubscribe(*this, &Application::onKeyPress);
    SceneManager::destroy();
    delete window;
    Renderer::destroy();
}

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

void Application::onWindowClose(const WindowCloseEvent &e)
{
    running = false;
}

void Application::onFramebufferResize(const FramebufferResizeEvent& e)
{
    if (Window::getActive().isMinimized())
        return;
    update();
    update();
}

void Application::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Keys::ESCAPE:
        Dispatcher::post(WindowCloseEvent(e.window));
        break;
    case Keys::F11:
        e.window.setFullscreen(!e.window.isFullscreen());
        break;
    }
}
