#include "sandbox_app.h"

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = makeShared<Scene>();

    Window::setSize({ 800, 600 });
    camera = makeShared<Entity>(scene->createEntity());
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    image = makeShared<Image>("data/images/test.png");
    uniform_buffer = makeShared<UniformBuffer>(sizeof(Graphics::GlobalUniformData));

    shared<DescriptorSet> descriptor_set = makeShared<DescriptorSet>(*Resources::getMaterial("3D")->descriptor_set_layout);
    descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
        .addImage(1, *image)
        .build();

    shared<MaterialData> material_data = makeShared<MaterialData>(Resources::getMaterial("3D"), descriptor_set);

    circles.resize(65536);
    for (size_t i = 0; i < circles.size(); ++i)
    {
        circles[i] = makeShared<Entity>(scene->createEntity());
        circles[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(RNG::Float(), RNG::Float(), RNG::Float() + 0.05f) * 50.0f));
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
