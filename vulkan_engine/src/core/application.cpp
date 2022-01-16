#include "application.h"
#include "input.h"
#include "utils/delta.h"
#include "time.h"
#include "keys.h"
#include "gfx/graphics.h"
#include "utils/timers.h"

Application::Application(const char* name, ApplicationCommandLineArgs args)
    : command_line_args(args), app_update(0.0)
{
    Log::init(); 
    VE_CORE_INFO("Started");

    Window::init();

    Input::init();
    
    Dispatcher::subscribe(this, &Application::onWindowClose);
    Dispatcher::subscribe(this, &Application::onKeyPress);

    Graphics::init(Window::getGLFWWindow());
}

Application::~Application()
{
    Dispatcher::unsubscribe(this, &Application::onWindowClose);
    Dispatcher::unsubscribe(this, &Application::onKeyPress);
    Window::cleanup();
    Graphics::cleanup();
    VE_CORE_INFO("Terminated");
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
    using namespace std::chrono_literals;
    Timers::every(0.5s, [this] { VE_CORE_TRACE("{0} FPS ({1:.4} ms)", app_update.getFPS(), (app_update.getDeltaTime() * 1000)); });

    while (running)
    {
        if (app_update.update())
        {
            if (!Window::isMinimized())
            {
                Time::dt = app_update.getDeltaTime();
                Time::frames = app_update.getFramesPassed();
                Time::runtime = app_update.getRuntime();

                Graphics::update(); 

                onUpdate();

                for (Layer *layer : layer_stack)
                    layer->onUpdate();

                Timers::update();
            }
            else
            {
                glfwWaitEvents();
            }

            glfwPollEvents();
        }
    }
}

void Application::onWindowClose(const WindowCloseEvent &e)
{
    VE_CORE_INFO("Window closed");
    running = false;
}

void Application::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Keys::ESCAPE:
        running = false;
        break;
    case Keys::F11:
        Window::setFullscreen(!Window::isFullscreen());
        break;
    }
}
