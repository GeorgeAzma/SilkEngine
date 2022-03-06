#include "application.h"
#include "input/input.h"
#include "utils/delta.h"
#include "time.h"
#include "input/keys.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "utils/timers.h"
#include "scene/resources.h"
#include "gfx/window/swap_chain.h"
#include "gfx/window/window.h"

Application::Application(const char* name, ApplicationCommandLineArgs args)
    : command_line_args(args), app_update(0.0)
{
    Log::init(); 
    SK_INFO("Started");
    Window::init();
    Dispatcher::subscribe(this, &Application::onWindowClose);
    Dispatcher::subscribe(this, &Application::onWindowResize);
    Dispatcher::subscribe(this, &Application::onKeyPress);
    Input::init();
    Graphics::init();
    Resources::init();
}

Application::~Application()
{
    Dispatcher::unsubscribe(this, &Application::onWindowClose);
    Dispatcher::unsubscribe(this, &Application::onWindowResize);
    Dispatcher::unsubscribe(this, &Application::onKeyPress);
    Window::cleanup();
    Resources::cleanup();
    Graphics::cleanup();
    SK_INFO("Terminated");
}

void Application::pushLayer(Layer *layer)
{
    layer_stack.pushLayer(layer);
    layer->onAttach();
}

void Application::pushOverlay(Layer *layer)
{
    layer_stack.pushOverlay(layer);
    layer->onAttach();
}

void Application::run()
{
    while (running)
    {
        update();
    }
}

void Application::update()
{
    if (app_update.update())
    {
        if (!Window::isMinimized())
        {
            Time::dt = app_update.getDeltaTime();
            Time::frame = app_update.getFramesPassed();
            Time::runtime = app_update.getRuntime();

            onUpdate();

            for (Layer* layer : layer_stack)
                layer->onUpdate();

            Timers::update();
            Graphics::update();
            Input::update();
        }
        else
        {
            glfwWaitEvents();
        }
        glfwPollEvents();
    }
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
