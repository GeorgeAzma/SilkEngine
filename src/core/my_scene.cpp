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

#include "my_scene.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/circle_mesh.h"
#include "scene/meshes/rounded_rectangle_mesh.h"
#include "gfx/particle_system.h"

Cooldown c(100ms);

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
}

void MyScene::onUpdate()
{
    if (Window::getActive().isKeyPressed(Keys::F2))
        RenderContext::screenshot("screenshot.png");

    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));

    shared<Mesh> mesh = makeShared<Mesh>(CircleMesh(32));
    shared<Mesh> mesh2 = makeShared<Mesh>(RoundedRectangleMesh(8));
    size_t j = 100;
    float r = 12;
    for (int i = 0; i < 10000; ++i)
    {
        //DebugRenderer::color(Colors(i % (1 + int(Colors::TRANSPARENT))));
        DebugRenderer::image(Image::get("Cursor"));
        DebugRenderer::mesh(mesh, (i % j) * r * 2 + 30.0f, i / j * r * 4 + 30.0f, r, r);
        DebugRenderer::mesh(mesh, (i % j) * r * 2 + 30.0f, i / j * r * 4 + r * 2 + 30.0f, r, r);
    }
}

void MyScene::onStop()
{
}
