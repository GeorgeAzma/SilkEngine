#include "silk_engine/scene/resources.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/core/input/keys.h"
#include "silk_engine/core/input/mouse_buttons.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/gfx/debug_renderer.h"
#include "silk_engine/utils/cooldown.h"
#include "silk_engine/gfx/render_context.h"

#include "my_scene.h"
#include "scene/meshes/line_mesh.h"
#include "scene/meshes/circle_mesh.h"

Cooldown c(200ms);

void MyScene::onStart()
{
    Window::getActive().setSize({ 1280, 720 });
    camera = createEntity();
    camera->add<CameraComponent>();
    camera->add<ScriptComponent>().bind<CameraController>();
    camera->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, -5.0f);

    int n = 500;
    std::vector<vec2> p(n);
    for (int i = 0; i < n; ++i)
    {
        p[i].x = i / float(n - 1);
        p[i].y = cos(pi<float>() * 2.0 * 3.0 * p[i].x + Time::runtime) * 100.0f + 100.0f;
        p[i].x *= Window::getActive().getWidth();
    }
    shared<Mesh> line = makeShared<Mesh>(CircleMesh(32));
    DebugRenderer::InstanceData data{};
    data.transform = glm::scale(data.transform, vec3(8));
    for (int i = 0; i < 10000; ++i)
    {
        DebugRenderer::createInstance(line, data, Resources::get<GraphicsPipeline>("2D"));
        data.transform = glm::translate(data.transform, vec3(1, i % 100 == 0 ? 1 : 0, 0));
        if (i % 100 == 0)
        {
            data.transform[3][0] = 0.0f;
        }
        data.color = Color(Colors(i % (1 + int(Colors::TRANSPARENT))));
    }
}

void MyScene::onUpdate()
{
    if (Window::getActive().isKeyPressed(Keys::F2))
        RenderContext::screenshot(path(std::format("res/images/screenshots/screenshot.png")));
    if (Window::getActive().isKeyPressed(Keys::R))
        Resources::reloadShaders();

    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));
    
}

void MyScene::onStop()
{
}
