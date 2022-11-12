#include "application.h"
#include "input/input.h"
#include "time.h"
#include "input/keys.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "scene/resources.h"
#include "gfx/window/window.h"
#include "gfx/window/swap_chain.h"
#include "scene/scene_manager.h"
#include "gfx/renderer.h"
#include "gfx/descriptors/descriptor_allocator.h"
#include "gfx/devices/logical_device.h"
#include "scene/camera/camera.h"
#include <GLFW/glfw3.h>

Application::Application(ApplicationCommandLineArgs args)
    : command_line_args(args)
{
    Log::init(); 
    SK_INFO("Started");
    Window::init();
    Input::init();
    Graphics::init(*this);
    Resources::init();
    Renderer::init();
    Dispatcher::subscribe(this, &Application::onWindowClose);
    Dispatcher::subscribe(this, &Application::onWindowResize);
    Dispatcher::subscribe(this, &Application::onKeyPress);
}

Application::~Application()
{
    Dispatcher::unsubscribe(this, &Application::onWindowClose);
    Dispatcher::unsubscribe(this, &Application::onWindowResize);
    Dispatcher::unsubscribe(this, &Application::onKeyPress);
    SceneManager::destroy();
    Window::destroy();
    Renderer::destroy();
    Resources::destroy();
    DescriptorAllocator::destroy();
    Graphics::destroy();
    glfwTerminate();
    SK_INFO("Terminated");
}

void Application::run()
{
    fully_initialized = true;
    while (running)
        update();

    Graphics::logical_device->wait();
}

void Application::update()
{
    if (Window::isMinimized())
    {
        glfwWaitEvents();
        return;
    }

    glfwPollEvents();

    if (Window::isMinimized())
    {
        glfwWaitEvents();
        return;
    }

    Renderer::waitForPreviousFrame();

    Renderer::reset();
    onUpdate();
    SceneManager::update();

    Graphics::update();
    Renderer::render(SceneManager::getActive().get() ? SceneManager::getActive()->getMainCamera() : nullptr);

    Window::update();
    Time::update();
}

void Application::onWindowClose(const WindowCloseEvent &e)
{
    SK_INFO("Window closed");
    running = false;
}

void Application::onWindowResize(const WindowResizeEvent& e)
{
    Graphics::physical_device->updateSurfaceCapabilities();
    Graphics::swap_chain->recreate();
    if (fully_initialized)
        update();
}

void Application::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Keys::ESCAPE:
        Dispatcher::post(WindowCloseEvent());
        break;
    case Keys::F11:
        Window::setFullscreen(!Window::isFullscreen());
        break;
    }
}
