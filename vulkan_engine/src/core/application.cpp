#include "application.h"
#include "../utils/delta.h"
#include "time.h"

Application::Application(const char *name, ApplicationCommandLineArgs args)
    : command_line_args(args), app_update(256)
{
    WindowProps props{};
    props.title = name;
    window = std::make_shared<Window>(props);
    Log::init();
    Dispatcher::subscribe(this, &Application::onWindowClose);
}

Application::~Application()
{
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
        if (app_update.update())
        {
            if (!window->isMinimized())
            {
                Time::dt = app_update.getElapsedTime();
                Time::frames = app_update.getFramesPassed();
                Time::runtime = app_update.getRuntime();
                for (Layer *layer : layer_stack)
                    layer->onUpdate();
            }
            window->update();
        }
    }
}

void Application::onWindowClose(const WindowCloseEvent &e)
{
    running = false;
}
