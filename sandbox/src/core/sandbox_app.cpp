#include "sandbox_app.h"

std::shared_ptr<Entity> circle;

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    Window::setSize({ 800, 600 });
    camera = std::make_shared<Entity>(scene.createEntity());
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();
    scene.onPlay();

    descriptor_set_layout = std::make_shared<DescriptorSetLayout>();
    descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    std::shared_ptr<Shader> shader(
        new Shader({
        "data/cache/shaders/test.vert.spv",
        "data/cache/shaders/test.frag.spv"
            })); //1.65ms

    image = std::make_shared<Image>("data/images/test.png");

    GraphicsPipelineProps graphics_pipeline_props{};
    graphics_pipeline = std::make_shared<GraphicsPipeline>(); //1.35ms
    graphics_pipeline->enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .addDescriptorSetLayout(*descriptor_set_layout)
        .setShader(shader)
        .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::MAT4, 1 } })
        .setSampleCount(Graphics::swap_chain->getSampleCount())
        .setRenderPass(Graphics::swap_chain->getRenderPass())
        .build();

    uniform_buffer = std::make_shared<UniformBuffer>(sizeof(Transforms));

    descriptor_set = std::make_shared<DescriptorSet>(*descriptor_set_layout, Graphics::swap_chain->getImages().size());
    descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
        .addImage(1, image.get())
        .build();

    circle = std::make_shared<Entity>(scene.createEntity());
    circle->addComponent<TransformComponent>(glm::mat4(1));
    circle->addComponent<MeshComponent>(std::shared_ptr<CircleMesh>(new CircleMesh()));
    circle->addComponent<RenderComponent>(descriptor_set_layout, descriptor_set, graphics_pipeline);
}

void SandboxApp::onUpdate()
{
    Graphics::beginFrame();
    //graphics_pipeline->bind();

    scene.onUpdate();

    //descriptor_set->bind(Graphics::swap_chain->getImageIndex());

    //vkCmdDrawIndexed(Graphics::active.command_buffer, indices.size(), instance_data.size(), 0, 0, 0);

    uniform_buffer->setData(&camera->getComponent<CameraComponent>().projection_view);

    Graphics::endFrame();
}

SandboxApp::~SandboxApp()
{
    Graphics::vulkanAssert(vkDeviceWaitIdle(*Graphics::logical_device));
    scene.onStop();
}

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
