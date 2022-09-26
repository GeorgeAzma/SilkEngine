#include "my_scene.h"
#include "scene/resources.h"
#include "core/input/input.h"
#include "core/input/keys.h"
#include "core/input/mouse_buttons.h"
#include "gfx/window/window.h"
#include "scene/camera/camera_controller.h"
#include "scene/components.h"
#include "gfx/renderer.h"
#include "utils/cooldown.h"

Cooldown c(200ms);

void MyScene::onStart()
{
    Window::setSize({ 1280, 720 });
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
    Input::lockMouse();
}

void MyScene::onUpdate()
{
    if (Input::isKeyPressed(Keys::F2))
        Graphics::screenshot(std::format("data/images/screenshots/screenshot.png"));
    if (Input::isKeyPressed(Keys::R))
        Resources::reloadShaders();

    if (c())
        Window::setTitle(std::format("Vulkan - {0} FPS ({1:.4} ms) | {2}x{3}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getWidth(), Window::getHeight()));

    Renderer::color(Colors::WHITE);
    Renderer::draw(Resources::get<GraphicsPipeline>("3D"), Resources::get<Mesh>("Cube"), mat4(1));
    Renderer::roundedRectangle(10.0f, 10.0f, 100.0f, 30.0f);
    Renderer::roundedRectangle(300.0f, 10.0f, 100.0f, 30.0f);
    Renderer::rectangle(100.0f, 30.0f, 20.0f, 40.0f);
    Renderer::circle(10.0f, 300.0f, 32.0f);
    Renderer::triangle(100.0f, 100.0f, 40.0f);
}

void MyScene::onStop()
{
    image = nullptr;
}
