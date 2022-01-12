#include "sandbox_app.h"

Scene scene;
std::shared_ptr<Entity> camera;

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    window->setSize({ 800, 600 });
    camera = std::make_shared<Entity>(scene.createEntity());
    camera->addComponent<TransformComponent>();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();
    scene.onPlay();
}

void SandboxApp::onUpdate()
{
    scene.onUpdate();
}

SandboxApp::~SandboxApp()
{
    scene.onStop();
}

//CREATE APP
Application *createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
