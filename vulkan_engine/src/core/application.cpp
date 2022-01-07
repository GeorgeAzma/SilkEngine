#include "application.h"
#include "utils/delta.h"
#include "time.h"
#include "gfx/graphics.h"

Application::Application(const char* name, ApplicationCommandLineArgs args)
    : command_line_args(args), app_update(0.0)
{
    Log::init(); 
    VE_CORE_INFO("Started");

    WindowProps props{};
    props.title = name;
    window = std::make_shared<Window>(props);
    
    Dispatcher::subscribe(this, &Application::onWindowClose);

    Graphics::init(window->getGLFWWindow());
}

Application::~Application()
{
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
    while (running)
    {
        if (app_update.update())
        {
            if (!window->isMinimized())
            {
                Time::dt = app_update.getDeltaTime();
                Time::frames = app_update.getFramesPassed();
                Time::runtime = app_update.getRuntime();

                if (Time::frames % 1024 == 0)
                {
                    VE_CORE_TRACE("{0} FPS ({1:.4} ms)", app_update.getFPS(), (app_update.getDeltaTime() * 1000));
                }

                Graphics::update();

                for (Layer *layer : layer_stack)
                    layer->onUpdate();
            }

            window->update();
        }
    }
}

void Application::onWindowClose(const WindowCloseEvent &e)
{
    VE_CORE_INFO("Window closed");
    running = false;
}
