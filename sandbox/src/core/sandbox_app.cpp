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
    entities.resize(0);
    for (size_t i = 0; i < entities.size(); ++i)
    {
        entities[i] = scene->createEntity();
        entities[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(i % (size_t)sqrt(entities.size()), i / (size_t)sqrt(entities.size()), RNG::Float() * sqrt(entities.size()) + 1.0f)));
        entities[i]->addComponent<SpriteComponent>((uint32_t)RNG::Bool());
        entities[i]->addComponent<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        entities[i]->addComponent<ModelComponent>(Resources::getModel("Backpack"));
        //entities[i]->addComponent<MeshComponent>(makeShared<RenderedInstance>(circle));
    }

    entities.emplace_back(scene->createEntity());
    entities.back()->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1)));
    entities.back()->addComponent<TextComponent>("Quick brown fox jumped over a lazy dog");
    Light light{};
    light.color = glm::vec3(4);
    entities.back()->addComponent<LightComponent>(light);
    //entities.back()->addComponent<MeshComponent>(makeShared<RenderedInstance>(rectangle));

    scene->onPlay();
}

void SandboxApp::onUpdate()
{  
    if (Input::isKeyDown(Keys::X) && entities.size())
    {
        entities.erase(entities.begin());
    }
    if (Input::isKeyDown(Keys::Z))
    {
        entities.emplace_back(scene->createEntity());
        entities.back()->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float(), RNG::Float(), RNG::Float()) * 20.0f));
        entities.back()->addComponent<ModelComponent>(Resources::getModel("Backpack"));
        //entities.back()->addComponent<MeshComponent>(makeShared<RenderedInstance>(Resources::getMesh("Circle")));
    }
    for (size_t i = 0; i < entities.size(); ++i)
    {
        glm::mat4& transform = entities[i]->getComponent<TransformComponent>();
        transform = glm::translate(transform, (glm::vec3(RNG::Float(), RNG::Float(), RNG::Float()) - 0.5f) * 100.0f * (float)Time::dt);
        scene->updateComponent<TransformComponent>(*entities[i]);
    }
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
