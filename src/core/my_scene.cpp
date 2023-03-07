#include "silk_engine/scene/resources.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/core/input/keys.h"
#include "silk_engine/core/input/mouse_buttons.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/scene/camera/camera_controller.h"
#include "silk_engine/scene/components.h"
#include "silk_engine/gfx/renderer.h"
#include "silk_engine/utils/cooldown.h"

#include "my_scene.h"

Cooldown c(200ms);

void MyScene::onStart()
{
    Window::getActive().setSize({ 1280, 720 });
    camera = createEntity();
    camera->add<CameraComponent>();
    camera->add<ScriptComponent>().bind<CameraController>();
    camera->get<CameraComponent>().camera.position = vec3(0.0f, 0.0f, -5.0f);

    Image::Props props{ 32, 32, 1, 1, Image::Format::RGBA };
    props.tiling = VK_IMAGE_TILING_LINEAR;
    image = makeShared<Image>(props);
    pixels.resize(image->getPixelCount());
    for (size_t i = 0; i < image->getWidth(); ++i)
        for (size_t j = 0; j < image->getHeight(); ++j)
            pixels[i + j * image->getWidth()] = (i + j % 2) % (RNG::Int() % 2 + 2) ? u8vec4{ 255, 0, 255, 255 } : u8vec4{ 0, 255, 0, 255 };

    image->setData(pixels.data());
    Renderer::addLight(Light{ .position = vec3(100.0f, 100.0f, -100.0f), .color = vec3(3)});
    Renderer::addLight(Light{ .position = vec3(-100.0f, 100.0f, 0.0f), .color = vec3(3) });
    Renderer::addLight(Light{ .position = vec3(0.0f, -100.0f, 0.0f), .color = vec3(3) });
    Renderer::addLight(Light{ .position = vec3(0.0f, 0.0f, 100.0f), .color = vec3(3) });
    Window::getActive().setCursorMode(CursorMode::LOCKED);
}

void MyScene::onUpdate()
{
    if (Window::getActive().isKeyPressed(Keys::F2))
        Graphics::screenshot(path(std::format("res/images/screenshots/screenshot.png")));
    if (Window::getActive().isKeyPressed(Keys::R))
        Resources::reloadShaders();

    if (c())
        Window::getActive().setTitle(std::format("Vulkan - {} FPS ({:.4} ms) | {}x{}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getActive().getWidth(), Window::getActive().getHeight()));
    
    Renderer::color(Colors::WHITE);
    int n = 100;
    std::vector<vec2> p(n);
    for (int i = 0; i < n; ++i)
    {
        p[i].x = i / float(n - 1);
        p[i].y = cos(pi<float>() * 2.0 * 3.0 * p[i].x) * 100.0f + 200.0f;
        p[i].x *= Window::getActive().getWidth();
    }
    Renderer::line(p, 4.0f);
}

void MyScene::onStop()
{
    image = nullptr;
}
