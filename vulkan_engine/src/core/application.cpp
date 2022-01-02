#include "application.h"
#include "../utils/delta.h"
#include "time.h"

Application::Application(const std::string &name, ApplicationCommandLineArgs args)
    : commandLineArgs(args), appUpdate(256)
{
    WindowProps props{};
    props.title = name;
    window = std::make_shared<Window>(props);
    Log::init();
}

Application::~Application()
{
}

void Application::pushLayer(Layer *layer)
{
    layerStack.pushLayer(layer);
    layer->onAttach();
}

void Application::pushOverlay(Layer *layer)
{
    layerStack.pushOverlay(layer);
    layer->onAttach();
}

void Application::run()
{
    Delta deltaTime(runtime);
    while (running)
    {
        if (appUpdate.update())
        {
            if (!minimized)
            {
                time.dt = appUpdate.getElapsedTime();
                time.frames = appUpdate.getFramesPassed();
                time.runtime = appUpdate.getRuntime();
                for (Layer *layer : layerStack)
                    layer->onUpdate();
            }
            window->update();
        }
    }
}
