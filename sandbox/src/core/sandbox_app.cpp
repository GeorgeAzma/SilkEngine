#include "sandbox_app.h"
   
SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeUnique<Scene>();  
    
    Timers::every(100ms, 
        [this] 
        { 
            Window::setTitle(fmt::format("Vulkan - {0} FPS ({1:.4} ms) | {2}x{3}", int(1.0 / Time::dt), (Time::dt * 1000), Window::getWidth(), Window::getHeight()));
        });

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    auto rectangle = Resources::getMesh("Rectangle");
    auto circle = Resources::getMesh("Circle");
    entities.resize(10000);
    for (size_t i = 0; i < entities.size(); ++i)
    {
        entities[i] = scene->createEntity();
        entities[i]->addComponent<MaterialComponent>(Resources::getShaderEffect("2D"));
        auto t = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(Window::getWidth() * RNG::Float(), Window::getHeight() * RNG::Float(), 0.0f)), glm::vec3(4.0f, 4.0f, 0.0f));
        entities[i]->addComponent<TransformComponent>(t);
        entities[i]->addComponent<ImageComponent>(Resources::getImage(RNG::Bool() ? "Test2" : "Test1"));
        entities[i]->addComponent<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        entities[i]->addComponent<MeshComponent>(circle);
        //entities[i]->addComponent<ModelComponent>(Resources::getModel("Backpack"));
    }

    entities.emplace_back(scene->createEntity());
    entities.back()->addComponent<TransformComponent>(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)), glm::vec3(50, 50, 0)));
    entities.back()->addComponent<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    entities.back()->addComponent<TextComponent>("Quick brown fox jumped over a lazy dog");
    entities.back()->updateComponent<TextComponent>([](TextComponent& text) { text.text = "Press F2"; });

    scene->onPlay();
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
        entities.back()->addComponent<MaterialComponent>(Resources::getShaderEffect("3D"));
        entities.back()->addComponent<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        entities.back()->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float(), RNG::Float(), RNG::Float()) * 20.0f));
        entities.back()->addComponent<ModelComponent>(Resources::getModel("Backpack"));
        Light light{};
        light.color = glm::vec3(1);
        entities.back()->addComponent<LightComponent>(light);
    }
    if (Input::isKeyPressed(Keys::F2))
    {
        Graphics::screenshot(fmt::format("data/images/screenshots/screenshot.png"));
    }

    //Resources::pool.forEach(entities.size(), 
    //    [&](size_t i) 
    //    {
    //        entities[i]->updateComponent<TransformComponent>([](TransformComponent& transform) {});
    //        entities[i]->updateComponent<ColorComponent>([](ColorComponent& color) { color.color = glm::vec4(RNG::Float(), RNG::Float(), RNG::Float(), 1.0f); });
    //    });
    //Resources::pool.wait();

    scene->onUpdate();
}

SandboxApp::~SandboxApp()
{
}

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
