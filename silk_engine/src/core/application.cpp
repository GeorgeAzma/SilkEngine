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
#include "scene/scene_manager.h"
#include "gfx/renderer.h"
#include "gfx/descriptors/descriptor_allocator.h"
#include "gfx/particle_system.h"

Application::Application(ApplicationCommandLineArgs args)
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
    Renderer::init();
    ParticleSystem::init();
}

Application::~Application()
{
    Dispatcher::unsubscribe(this, &Application::onWindowClose);
    Dispatcher::unsubscribe(this, &Application::onWindowResize);
    Dispatcher::unsubscribe(this, &Application::onKeyPress);
    Window::cleanup();
    ParticleSystem::cleanup();
    Renderer::cleanup();
    Resources::cleanup();
    DescriptorAllocator::cleanup();
    Graphics::cleanup();
    SK_INFO("Terminated");
}

void Application::run()
{
    while (running)
        update();
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
            
            Renderer::waitForPreviousFrame();

            Renderer::reset();
            Graphics::update();
            onUpdate();
            SceneManager::update();
            ParticleSystem::update();

            Renderer::begin(SceneManager::getActive().get() ? SceneManager::getActive()->getMainCamera() : nullptr);
            Renderer::render();
            ParticleSystem::render();
            Renderer::end();

            Timers::update();
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
