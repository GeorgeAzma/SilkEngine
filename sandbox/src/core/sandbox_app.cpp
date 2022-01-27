#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeShared<Scene>();

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    //Renderer::beginBatch();
    auto circle = Resources::getMesh("Circle");
    circles.resize(20);
    for (size_t i = 0; i < circles.size(); ++i)
    {
        circles[i] = scene->createEntity();
        circles[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float() * 1.5f, RNG::Float(), 0.05f) * 5.0f));
        circles[i]->addComponent<RenderComponent>(circle);
    }

    auto rectangle = Resources::getMesh("Rectangle");
    squares.resize(0);
    size_t square_root = std::sqrt(squares.size());
    for (size_t i = 0; i < squares.size(); ++i)
    {
        squares[i] = scene->createEntity();
        squares[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(i % square_root, i / square_root, 1.0f)));
        squares[i]->addComponent<SpriteComponent>((uint32_t)RNG::Bool());
        squares[i]->addComponent<ColorComponent>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        squares[i]->addComponent<RenderComponent>(rectangle);
    }

    //Font font("arial.ttf");

    scene->onPlay();

    //Renderer::endBatch();
}

void SandboxApp::onUpdate()
{  
    if (Input::isKeyDown(Keys::X))
        squares.pop_back();

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
