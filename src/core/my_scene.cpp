#include "silk_engine/gfx/window/window.h"
#include "silk_engine/core/input/keys.h"
#include "silk_engine/core/input/mouse_buttons.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/utils/cooldown.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/particle_system.h"

#include "my_scene.h"
#include "scene/meshes/mesh.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/circle_mesh.h"
#include "scene/meshes/rounded_rectangle_mesh.h"

Cooldown c(100ms);
shared<Mesh> mesh;
shared<Mesh> mesh2;
std::vector<shared<DebugRenderer::RenderedInstance>> instances;

void MyScene::onStart()
{
    Window::getActive().setSize({ 1280, 720 });
    camera = createEntity();
    camera->add<CameraComponent>();
    camera->add<ScriptComponent>().bind<CameraController>();
    camera->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, -3.0f);

    Image::add("Cursor", makeShared<Image>("cursors/cursor.png"));
    Image::get("Cursor")->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    RenderContext::execute();
    mesh = makeShared<Mesh>(CircleMesh(32));
    mesh2 = makeShared<Mesh>(RoundedRectangleMesh(8));
    DebugRenderer::InstanceData data{};
    data.color = DebugRenderer::getActive().color;
    data.image_index = 0;
    size_t j = 100;
    float r = 12;
    DebugRenderer::image(Image::get("Cursor"));
    for (int i = 0; i < 1'000'000; ++i)
    {
        data.transform = {
                r, 0, 0, 0,
                0, r, 0, 0,
                0, 0, 1, 0,
                (i % j) * r * 2 + 30.0f, i / j * r * 4 + 30.0f, DebugRenderer::getActive().depth, 1
        };
        if (DebugRenderer::getActive().transformed)
            data.transform *= DebugRenderer::getActive().transform;
        instances.emplace_back(DebugRenderer::createInstance(mesh, data, GraphicsPipeline::get("2D"), DebugRenderer::getActive().images));
    }
}

void MyScene::onUpdate()
{
    if (Window::getActive().isKeyPressed(Keys::F2))
        RenderContext::screenshot("screenshot.png");

    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));

    size_t j = 100;
    float r = 12;
    //DebugRenderer::image(Image::get("Cursor"));
    for (int i = 0; i < 100000; ++i)
    {
        //DebugRenderer::color(Colors(i % (1 + int(Colors::TRANSPARENT))));
        //DebugRenderer::mesh(mesh, (i % j) * r * 2 + 30.0f, i / j * r * 4 + 30.0f, r, r);
        //DebugRenderer::mesh(mesh2, (i % j) * r * 2 + 30.0f, i / j * r * 4 + r * 2 + 30.0f, r, r);
    }
}

void MyScene::onStop()
{
}
