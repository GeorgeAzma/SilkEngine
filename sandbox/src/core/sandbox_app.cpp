#include "sandbox_app.h"
#include "utils/timers.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/images/image2D.h"
#include "scene/scene_manager.h"
#include "scene/model.h"
#include "scene/components.h"
#include "scene/resources.h"
#include "scene/camera/camera_controller.h"
#include "scene/camera/camera.h"
#include "scene/meshes/circle_mesh.h"
#include "scene/meshes/rectangle_mesh.h"
#include "core/input/input.h"
#include "core/input/keys.h"
#include <glm/gtc/matrix_transform.hpp>
   
SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{ 
    scene = makeShared<Scene>();  
    SceneManager::add(scene);
    SceneManager::switchTo(scene);

    Resources::addImage("Test1", [] { return makeShared<Image2D>("test1.png"); });
    Resources::addImage("Test2", [] { return makeShared<Image2D>("test2.png"); });
    Resources::addModel("Backpack", [] { return makeShared<Model>("backpack/backpack.obj"); });
    
    Timers::every(100ms, 
        [this] 
        { 
            Window::setTitle(fmt::format("Vulkan - {0} FPS ({1:.4} ms) | {2}x{3}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getWidth(), Window::getHeight()));
        });   

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->add<CameraComponent>();
    camera->add<ScriptComponent>().bind<CameraController>();

    auto rectangle = Resources::getMesh("Rectangle");
    auto circle = Resources::getMesh("Circle");
    entities.resize(100000);
    for (size_t i = 0; i < entities.size(); ++i)
    {
        entities[i] = scene->createEntity();
        entities[i]->add<MaterialComponent>(Resources::getGraphicsPipeline("2D"));
        auto t = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(Window::getWidth() * RNG::Float(), Window::getHeight() * RNG::Float(), 0.0f)), glm::vec3(2.0f, 2.0f, 0.0f));
        entities[i]->add<TransformComponent>(t);
        entities[i]->add<ImageComponent>(Resources::getImage(RNG::Bool() ? "Test2" : "Test1"));
        entities[i]->add<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        entities[i]->add<MeshComponent>(rectangle);
        //entities[i]->add<ModelComponent>(Resources::getModel("Backpack"));
    }

    entities.emplace_back(scene->createEntity());
    entities.back()->add<TransformComponent>(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)), glm::vec3(50, 50, 0)));
    entities.back()->add<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    entities.back()->add<TextComponent>("Quick brown fox jumped over a lazy dog");
    entities.back()->update<TextComponent>([](TextComponent& text) { text.text = "Press F2"; });
}

void SandboxApp::onUpdate()
{  
    if (Input::isKeyPressed(Keys::X) && entities.size())
    {
        entities.erase(entities.begin());
    }
    if (Input::isKeyPressed(Keys::Z)) 
    {
        entities.emplace_back(scene->createEntity());
        entities.back()->add<MaterialComponent>(Resources::getGraphicsPipeline("Lit 3D"));
        entities.back()->add<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        entities.back()->add<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float(), RNG::Float(), RNG::Float()) * 20.0f));
        entities.back()->add<ModelComponent>(Resources::getModel("Backpack"));
        Light light{};
        light.color = glm::vec3(1);
        entities.back()->add<LightComponent>(light);
    }
    if (Input::isKeyPressed(Keys::F2))
    {
        Graphics::screenshot(fmt::format("data/images/screenshots/screenshot.png"));
    }

    //Resources::pool.forEach(entities.size(), 
    //    [&](size_t i) 
    //    {
    //        entities[i]->update<TransformComponent>([](TransformComponent& transform) {});
    //        entities[i]->update<ColorComponent>([](ColorComponent& color) { color.color = glm::vec4(RNG::Float(), RNG::Float(), RNG::Float(), 1.0f); });
    //    });
    //Resources::pool.wait();
}

SandboxApp::~SandboxApp()
{
    SceneManager::remove(scene);
}

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
