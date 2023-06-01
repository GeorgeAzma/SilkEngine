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
    Window::get().setSize({ 1280, 720 });

    render_graph = makeShared<RenderGraph>();
    auto& geometry = render_graph->addPass("Geometry");
    auto& color = geometry.addAttachment("Color", Image::Format::BGRA, RenderContext::getPhysicalDevice().getMaxSampleCount());
    color.setClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
    auto& depth = geometry.addAttachment("Depth", Image::Format::DEPTH24_STENCIL, RenderContext::getPhysicalDevice().getMaxSampleCount());
    depth.setClearDepthStencil({ 1.0f, 0 });
    geometry.setRenderCallback([&](const RenderGraph& render_graph) 
    { 
        world->render();
        DebugRenderer::render(); 
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
        Window::get().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::get().getWidth(), Window::get().getHeight()));


    DebugRenderer::reset();
    world->update();
    DebugRenderer::color(Colors::WHITE);
    static RenderGraph::Statistics stats{};
    DebugRenderer::text(std::string("Vertex Invocations: ") + std::to_string(stats.vertex_invocations / 1000) + "K", 16.0f, 32.0f, 24.0f);
    DebugRenderer::text(std::string("Geometry Primitives: ") + std::to_string(stats.geometry_primitives / 1000) + "K", 16.0f, 64.0f, 24.0f);
    DebugRenderer::text(std::string("Fragment Invocations: ") + std::to_string(stats.fragment_invocations / 1000) + "K", 16.0f, 96.0f, 24.0f);
    DebugRenderer::text(std::string("Compute Invocations: ") + std::to_string(stats.compute_invocations / 1000) + "K", 16.0f, 128.0f, 24.0f);
    render_graph->render(&stats);
    DebugRenderer::update(Scene::getActive()->getMainCamera());
}

void MyScene::onStop()
{
    DebugRenderer::destroy();
    render_graph = nullptr;
}
