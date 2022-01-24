#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeShared<Scene>();

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    auto white = Resources::getImage("White");
    auto null = Resources::getImage("Null");
    auto image1 = Resources::getImage("Test1");
    auto image2 = Resources::getImage("Test2");

    shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>(*Resources::getMaterial("3D")->descriptor_set_layout);
    descriptor_set->addBuffer(0, { *Graphics::global_uniform, 0, VK_WHOLE_SIZE })
        .addImages(1, { *white, *null, *white, *white,
            *white, *white, *white, *white, 
            *white, *white, *white, *white, 
            *white, *white, *white, *white, 
            *white, *white, *white, *white, 
            *white, *white, *white, *white, 
            *white, *white, *white, *white, 
            *white, *white, *white, *white })
        .build();
    material_data = makeShared<MaterialData>(Resources::getMaterial("3D"), descriptor_set);

    auto circle = Resources::getMesh("Circle");
    circles.resize(0);
    for (size_t i = 0; i < circles.size(); ++i)
    {
        circles[i] = scene->createEntity();
        circles[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float() * 1.5f, RNG::Float(), 0.05f) * 5.0f));
        circles[i]->addComponent<RenderComponent>(circle, material_data);
    }

    auto rectangle = Resources::getMesh("Rectangle");
    squares.resize(28);
    for (size_t i = 0; i < squares.size(); ++i)
    {
        squares[i] = scene->createEntity();
        squares[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(i, 0.0f, 1.0f)));
        squares[i]->addComponent<SpriteComponent>((uint32_t)RNG::Bool());
        squares[i]->addComponent<RenderComponent>(rectangle, material_data);
    }

    scene->onPlay();
}

void SandboxApp::onUpdate()
{  
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
