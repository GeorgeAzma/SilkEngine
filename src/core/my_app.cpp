#include "silk_engine/gfx/window/glfw.h"
#include "silk_engine/core/input/input.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/renderer.h"
#include "silk_engine/core/event.h"
#include "silk_engine/core/input/keys.h"

#include "my_app.h"
#include "my_scene.h"

MyApp::MyApp()
{
    Input::init();
    GLFW::init();
    RenderContext::init("MyApp");

    window = new Window();
    Renderer::init();

    scene = makeShared<MyScene>();
    Scene::addScene(scene);

    Dispatcher<KeyPressEvent>::subscribe(*this, &MyApp::onKeyPress);
    Dispatcher<WindowCloseEvent>::subscribe(*this, &MyApp::onWindowClose);
    Dispatcher<FramebufferResizeEvent>::subscribe(*this, &MyApp::onFramebufferResize);
}

MyApp::~MyApp()
{
    Dispatcher<KeyPressEvent>::unsubscribe(*this, &MyApp::onKeyPress);
    Dispatcher<WindowCloseEvent>::unsubscribe(*this, &MyApp::onWindowClose);
    Dispatcher<FramebufferResizeEvent>::unsubscribe(*this, &MyApp::onFramebufferResize);

    Scene::destroyScenes();
    Renderer::destroy();
    delete window;
    RenderContext::destroy();
    GLFW::destroy();
    SK_INFO("Terminated");
}

void MyApp::onUpdate()
{
}

void MyApp::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Keys::ESCAPE:
        Dispatcher<WindowCloseEvent>::post(e.window);
        break;
    case Keys::F11:
        e.window.setFullscreen(!e.window.isFullscreen());
        break;
    }
}

void MyApp::onWindowClose(const WindowCloseEvent& e)
{
    stop();
}

void MyApp::onFramebufferResize(const FramebufferResizeEvent& e)
{
    update();
    update();
}