#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = std::make_shared<Scene>();

    Window::setSize({ 800, 600 });
    camera = std::make_shared<Entity>(scene->createEntity());
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    image = std::make_shared<Image>("data/images/test.png");
    uniform_buffer = std::make_shared<UniformBuffer>(sizeof(Graphics::GlobalUniformData));

    std::shared_ptr<DescriptorSet> descriptor_set = std::make_shared<DescriptorSet>(*Resources::getMaterial("3D")->descriptor_set_layout);
    descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
        .addImage(1, *image)
        .build();

    std::shared_ptr<MaterialData> material_data = std::make_shared<MaterialData>(Resources::getMaterial("3D"), descriptor_set);

    circles.resize(65536);
    for (size_t i = 0; i < circles.size(); ++i)
    {
        circles[i] = std::make_shared<Entity>(scene->createEntity());
        circles[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(.01f), glm::vec3(RNG::Float(), RNG::Float(), 0.02f) * 400.0f));
        circles[i]->addComponent<RenderComponent>(Resources::getMesh("Circle"), material_data);
    }
    
    scene->onPlay();
}

void SandboxApp::onUpdate()
{
    Graphics::beginFrame();

    scene->onUpdate();

    uniform_buffer->setData(&camera->getComponent<CameraComponent>().projection_view);

    Graphics::endFrame();
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
