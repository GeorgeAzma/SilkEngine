#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeShared<Scene>();

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    Renderer::beginBatch();
    auto rectangle = Resources::getMesh("Rectangle");
    auto circle = Resources::getMesh("Circle");
    entities.resize(1000000);
    for (size_t i = 0; i < entities.size(); ++i)
    {
        entities[i] = scene->createEntity();
        entities[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(i % (size_t)sqrt(entities.size()), i / (size_t)sqrt(entities.size()), 1.0f)));
        entities[i]->addComponent<SpriteComponent>((uint32_t)RNG::Bool());
        entities[i]->addComponent<ColorComponent>(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        entities[i]->addComponent<RenderComponent>(circle);
    }

    //entities.emplace_back(scene->createEntity());
    //entities.back()->addComponent<TextComponent>("Quick brown fox jumped over a lazy dog");
    //entities.back()->addComponent<RenderComponent>(rectangle);
    Renderer::endBatch();
    
    scene->onPlay();
}

//ThreadPool pool(6);
void SandboxApp::onUpdate()
{  
    if (Input::isKeyDown(Keys::X) && entities.size())
        entities.pop_back();

//#if 0
//    pool.forEach(entities.size(), 
//        [&](size_t i)
//        {           
//                glm::mat4& transform = entities[i]->getComponent<TransformComponent>();
//                transform = std::move(glm::translate(transform, glm::vec3(Time::dt * (RNG::Float() - 0.5f) * 10, Time::dt * (RNG::Float() - 0.5f) * 10, Time::dt * (RNG::Float() - 0.5f) * 10)));
//                scene->updateComponent<RenderComponent>(*entities[i]);
//        });
//#else
//   for (size_t i = 0; i < entities.size(); ++i)
//   {
//       glm::mat4& transform = entities[i]->getComponent<TransformComponent>();
//       transform = std::move(glm::translate(transform, glm::vec3(Time::dt * (RNG::Float() - 0.5f) * 10, Time::dt * (RNG::Float() - 0.5f) * 10, Time::dt * (RNG::Float() - 0.5f) * 10)));
//       scene->updateComponent<RenderComponent>(*entities[i]);
//   }
//#endif
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
