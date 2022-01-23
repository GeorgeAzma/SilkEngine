#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeShared<Scene>();

    Window::setSize({ 800, 600 });
    camera = scene->createEntity();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    image = makeShared<Image>("data/images/test.png");

    shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>(*Resources::getMaterial("3D")->descriptor_set_layout);
    descriptor_set->addBuffer(0, { *Graphics::global_uniform, 0, VK_WHOLE_SIZE })
        .addImage(1, *image)
        .build();
    material_data = makeShared<MaterialData>(Resources::getMaterial("3D"), descriptor_set);

    auto circle = Resources::getMesh("Circle");
    circles.resize(400000);
    for (size_t i = 0; i < circles.size(); ++i)
    {
        circles[i] = scene->createEntity();
        circles[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float() * 1.5f, RNG::Float(), 0.05f) * 1500.0f));
        circles[i]->addComponent<RenderComponent>(circle, material_data);
    }

    auto rectangle = Resources::getMesh("Rectangle");
    squares.resize(400000);
    for (size_t i = 0; i < squares.size(); ++i)
    {
        squares[i] = scene->createEntity();
        squares[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float() * 1.5f, RNG::Float(), 0.05f) * 1500.0f));
        squares[i]->addComponent<RenderComponent>(rectangle, material_data);
    }

    scene->onPlay();
}

void SandboxApp::onUpdate()
{
    if (RNG::Uint() % 64 == 0) 
    {
        circles.push_back(scene->createEntity()); 
        circles.back()->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float(), RNG::Float(), RNG::Float() + 0.05f) * 50.0f));
        circles.back()->addComponent<RenderComponent>(Resources::getMesh("Circle"), material_data);
    }
    if (Input::isKeyDown(Keys::X))
    {
        if (circles.size())
            circles.pop_back();
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
