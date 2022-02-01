#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeShared<Scene>();

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    auto rectangle = Resources::getMesh("Rectangle");
    auto circle = Resources::getMesh("Circle");
    entities.resize(100);
    for (size_t i = 0; i < entities.size(); ++i)
    {
        entities[i] = scene->createEntity();
        entities[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(i % (size_t)sqrt(entities.size()), i / (size_t)sqrt(entities.size()), RNG::Float() * 10 + 1.0f)));
        entities[i]->addComponent<SpriteComponent>((uint32_t)RNG::Bool());
        entities[i]->addComponent<ColorComponent>(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        entities[i]->addComponent<MeshComponent>(makeShared<RenderedInstance>(circle));
    }

    entities.emplace_back(scene->createEntity());
    entities.back()->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, -2.5f, 3)));
    entities.back()->addComponent<TextComponent>("Quick brown fox jumped over a lazy dog");
    entities.back()->addComponent<MeshComponent>(makeShared<RenderedInstance>(rectangle));
    
    entities.emplace_back(scene->createEntity());
    entities.back()->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2)));
    entities.back()->addComponent<ModelComponent>(Resources::getModel("Backpack"));
    
    scene->onPlay();
}

void SandboxApp::onUpdate()
{  
    if (Input::isKeyDown(Keys::X) && entities.size())
        entities.pop_back();

//   for (size_t i = 0; i < entities.size(); ++i)
//   {
//       glm::mat4& transform = entities[i]->getComponent<TransformComponent>();
//       transform = std::move(glm::translate(transform, glm::vec3(Time::dt * (RNG::Float() - 0.5f) * 10, Time::dt * (RNG::Float() - 0.5f) * 10, Time::dt * (RNG::Float() - 0.5f) * 10)));
//       scene->updateComponent<RenderComponent>(*entities[i]);
//   }
    scene->onUpdate();
}

SandboxApp::~SandboxApp()
{
    scene->onStop();
}

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
