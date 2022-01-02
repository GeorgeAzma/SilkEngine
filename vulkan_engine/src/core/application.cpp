#include "application.h"

Application::Application(const std::string &name, ApplicationCommandLineArgs args)
    : commandLineArgs(args)
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
        double time = glfwGetTime();
        Timestep timestep = time - m_LastFrameTime;
        m_LastFrameTime = time;

        if (!minimized)
        {
            for (Layer *layer : layerStack)
                layer->OnUpdate(timestep);
        }

        m_Window->OnUpdate();
    }
}
