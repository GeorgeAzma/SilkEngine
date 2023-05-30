#include "silk_engine/core/input/input.h"
#include "silk_engine/core/event.h"
#include "silk_engine/scene/scene.h"
#include "silk_engine/gfx/window/glfw.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/window/surface.h"
#include "silk_engine/gfx/window/swap_chain.h"
#include "silk_engine/gfx/buffers/framebuffer.h"
#include "silk_engine/gfx/fence.h"
#include "silk_engine/gfx/semaphore.h"
#include "silk_engine/gfx/material.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/gfx/pipeline/render_pass.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/pipeline/render_graph/render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"

#include "my_app.h"
#include "my_scene.h"

MyApp::MyApp()
{
    GLFW::init();
    Input::init();
    RenderContext::init("MyApp");

    window = makeShared<Window>();

    scene = makeShared<MyScene>();
    Scene::setActive(scene.get());

    Dispatcher<KeyPressEvent>::subscribe(*this, &MyApp::onKeyPress);
    Dispatcher<WindowCloseEvent>::subscribe(*this, &MyApp::onWindowClose);
    Dispatcher<FramebufferResizeEvent>::subscribe(*this, &MyApp::onFramebufferResize);
}

MyApp::~MyApp()
{
    Dispatcher<KeyPressEvent>::unsubscribe(*this, &MyApp::onKeyPress);
    Dispatcher<WindowCloseEvent>::unsubscribe(*this, &MyApp::onWindowClose);
    Dispatcher<FramebufferResizeEvent>::unsubscribe(*this, &MyApp::onFramebufferResize);

    scene = nullptr;
    window = nullptr;
    RenderContext::destroy();
    GLFW::destroy();
    SK_INFO("Terminated");
}

void MyApp::onUpdate()
{
    if (Scene::getActive())
        Scene::getActive()->update();
    RenderContext::update();
}

void MyApp::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Key::ESCAPE:
        e.window.close();
        Dispatcher<WindowCloseEvent>::post(e.window);
        break;
    case Key::F11:
        e.window.setFullscreen(!e.window.isFullscreen());
        break;
    case Key::L:
        if (e.window.getCursorMode() == CursorMode::LOCKED)
            e.window.setCursorMode(CursorMode::NORMAL);
        else if (e.window.getCursorMode() == CursorMode::NORMAL)
            e.window.setCursorMode(CursorMode::LOCKED);
        break;
    }
}

void MyApp::onWindowClose(const WindowCloseEvent& e)
{
    if (Window::getWindows().empty())
        stop();
}

void MyApp::onFramebufferResize(const FramebufferResizeEvent& e)
{
    update();
    update();
}