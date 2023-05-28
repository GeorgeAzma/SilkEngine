#include "silk_engine/gfx/window/window.h"
#include "silk_engine/core/input/input.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/utils/cooldown.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/pipeline/render_graph/render_graph.h"

#include "my_scene.h"
#include "world/world.h"

void MyScene::onStart()
{
    Window::getActive().setSize({ 1280, 720 });

    render_graph = makeShared<RenderGraph>();
    auto& geometry = render_graph->addPass("Geometry");
    auto& color = geometry.addAttachment("Color", Image::Format::BGRA, RenderContext::getPhysicalDevice().getMaxSampleCount());
    color.setClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    auto& depth = geometry.addAttachment("Depth", Image::Format::DEPTH24_STENCIL, RenderContext::getPhysicalDevice().getMaxSampleCount());
    depth.setClearDepthStencil({ 1.0f, 0 });
    geometry.setRenderCallback([&](const RenderGraph& render_graph) 
    { 
        DebugRenderer::render(); 
        world->render();
    });
    render_graph->build("Color");
    RenderContext::setRenderGraph(render_graph);

    DebugRenderer::init();

    world = makeShared<World>();
}

void MyScene::onUpdate()
{
    if (Input::isKeyPressed(Key::F2))
        RenderContext::screenshot("screenshot.png");

    static Cooldown c(100ms);
    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));

    world->update();
    render_graph->render();
}

void MyScene::onStop()
{
}
