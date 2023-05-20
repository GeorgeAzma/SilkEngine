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
#include "silk_engine/gfx/pipeline/render_graph.h"
#include "silk_engine/gfx/pipeline/render_pass.h"

#include "my_app.h"
#include "my_scene.h"

MyApp::MyApp()
{
    GLFW::init();
    Input::init();
    RenderContext::init("MyApp");

    window = makeUnique<Window>();

    render_graph = makeUnique<RenderGraph>();
    auto& geometry = render_graph->addPass("Geometry");
    auto& color = geometry.addAttachment("Color", Image::Format::BGRA, RenderContext::getPhysicalDevice().getMaxSampleCount(), VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f });
    auto& depth = geometry.addAttachment("Depth", Image::Format::DEPTH24_STENCIL, RenderContext::getPhysicalDevice().getMaxSampleCount(), VkClearDepthStencilValue{ 1.0f, 0 });

    geometry.setRenderCallback([&](const RenderGraph& render_graph)
        {
            DebugRenderer::render();
        });

    auto& post_process = render_graph->addPass("Post Process");
    auto& present_source = post_process.addAttachment("Present Source", Image::Format::BGRA, VK_SAMPLE_COUNT_1_BIT, { &color });
    post_process.setRenderCallback([&](const RenderGraph& render_graph)
        {
            auto& attachment = render_graph.getAttachment("Color");
            material->set("image", *attachment);
            material->bind();
            RenderContext::getCommandBuffer().draw(3);
        });

    render_graph->addRoot(present_source);
    render_graph->build();
    render_graph->print();

    shared<GraphicsPipeline> graphics_pipeline = makeShared<GraphicsPipeline>();
    graphics_pipeline->setShader(makeShared<Shader>(std::vector<std::string_view>{ "screen", "image" }))
        .setRenderPass(*post_process.getRenderPass())
        .build();
    material = makeShared<Material>(graphics_pipeline);
    
    DebugRenderer::init(*geometry.getRenderPass());

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
    window = nullptr;
    RenderContext::destroy();
    GLFW::destroy();
    SK_INFO("Terminated");
}

void MyApp::onUpdate()
{
    DebugRenderer::reset();

    if (Scene::getActive())
    {
        Scene::getActive()->onUpdate();
        DebugRenderer::update(Scene::getActive()->getMainCamera());
    }

    render_graph->render();

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