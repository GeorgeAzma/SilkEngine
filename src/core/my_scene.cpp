#include "silk_engine/gfx/window/window.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/utils/cooldown.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/pipeline/graphics_pipeline.h"
#include "silk_engine/gfx/particle_system.h"
#include "silk_engine/scene/model.h"

#include "my_scene.h"
#include "scene/meshes/mesh.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/circle_mesh.h"
#include "scene/meshes/rounded_rectangle_mesh.h"

Cooldown c(100ms);
shared<Mesh> mesh;
shared<Mesh> mesh2;
shared<Model> model;
std::vector<shared<DebugRenderer::Renderable>> instances;

void MyScene::onStart()
{
    Window::getActive().setSize({ 1280, 720 });
    camera = createEntity();
    camera->add<CameraComponent>();
    camera->add<ScriptComponent>().bind<CameraController>();
    camera->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, 50.0f);

    Image::add("Cursor", makeShared<Image>("cursors/cursor.png"));
    Image::get("Cursor")->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    RenderContext::execute();
    mesh = makeShared<Mesh>(CircleMesh(32));
    mesh2 = makeShared<Mesh>(RoundedRectangleMesh(8));
    model = makeShared<Model>("backpack.glb");
    
    Light l{};
    l.color = vec3(4);
    l.position = vec3(100, 100, 100);
    DebugRenderer::addLight(l); 
    l.position = vec3(-100, 200, -100);
    DebugRenderer::addLight(l);
    l.position = vec3(-10, 200, 20);
    DebugRenderer::addLight(l);
    l.position = vec3(-10, -200, 20);
    DebugRenderer::addLight(l);
}

void MyScene::onUpdate()
{
    if (Window::getActive().isKeyPressed(Key::F2))
        RenderContext::screenshot("screenshot.png");

    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));

    //DebugRenderer::image(Image::get("Cursor"));
    //for (int i = 0; i < 100'000; ++i)
    //    DebugRenderer::mesh(mesh, (i % 100) * 12 * 2 + 30.0f + sin(Time::runtime * 4.0) * 32.0f, i / 100 * 12 * 4 + 30.0f, 12, 12);

    DebugRenderer::model(model, 0.0f, 0.0f, 0.0f, 0.15f, 0.15f, 0.15f);
}

void MyScene::onStop()
{
}
