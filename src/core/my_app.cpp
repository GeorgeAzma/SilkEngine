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

#include "my_app.h"
#include "my_scene.h"

MyApp::MyApp()
{
    GLFW::init();
    Input::init();
    RenderContext::init("MyApp");

    window = new Window();

    previous_frame_finished = new Fence(true);
    swap_chain_image_available = new Semaphore();
    render_finished = new Semaphore();

    shared<RenderPass> render_pass = shared<RenderPass>(new RenderPass({
           {
               {
                   { Image::Format(RenderContext::getPhysicalDevice().getDepthFormat()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, RenderContext::getPhysicalDevice().getMaxSampleCount() },
                   { Image::Format(Window::getActive().getSurface().getFormat().format), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderContext::getPhysicalDevice().getMaxSampleCount() }
               }, {}
           }
        }));

    DebugRenderer::init(*render_pass);
    render_passes.emplace_back(render_pass);

    for (auto& render_pass : render_passes)
        render_pass->resize(Window::getActive().getSwapChain());

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
    DebugRenderer::destroy();
    delete previous_frame_finished;
    delete swap_chain_image_available;
    delete render_finished;
    render_passes.clear();
    delete window;
    RenderContext::destroy();
    GLFW::destroy();
    SK_INFO("Terminated");
}

void MyApp::onUpdate()
{
    previous_frame_finished->wait();
    previous_frame_finished->reset();
    DebugRenderer::reset();

    if (Scene::getActive())
    {
        Scene::getActive()->onUpdate();
        DebugRenderer::update(Scene::getActive()->getMainCamera());
    }

    if (!Window::getActive().getSwapChain().acquireNextImage(*swap_chain_image_available))
    {
        Window::getActive().recreate();
        for (auto& render_pass : render_passes)
            render_pass->resize(Window::getActive().getSwapChain());
    }

    CommandBuffer& cb = RenderContext::getCommandBuffer();

    for (uint32_t render_pass_index = 0; render_pass_index < render_passes.size(); ++render_pass_index)
    {
        auto& render_pass = render_passes[render_pass_index];
        uint32_t width = render_pass->getFramebuffer()->getWidth();
        uint32_t height = render_pass->getFramebuffer()->getHeight();

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = height;
        viewport.width = float(width);
        viewport.height = -float(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cb.setViewport({ viewport });

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { width, height };
        cb.setScissor({ scissor });


        render_pass->begin();
        if (render_pass_index == 0)
        {
            DebugRenderer::render();
        }
        render_pass->end();
    }
    RenderContext::submit(previous_frame_finished, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }, { *swap_chain_image_available }, { *render_finished });

    if (!Window::getActive().getSwapChain().present(*render_finished))
    {
        Window::getActive().recreate();
        for (auto& render_pass : render_passes)
            render_pass->resize(Window::getActive().getSwapChain());
    }

    RenderContext::update();
}

void MyApp::onKeyPress(const KeyPressEvent& e)
{
    switch (e.key)
    {
    case Key::ESCAPE:
        Dispatcher<WindowCloseEvent>::post(e.window);
        break;
    case Key::F11:
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