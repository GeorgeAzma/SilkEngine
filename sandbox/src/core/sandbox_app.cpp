#include "sandbox_app.h"

std::vector<std::shared_ptr<Entity>> circles;
//std::shared_ptr<IndirectBuffer> indirect_buffer;

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{
    scene = std::make_shared<Scene>();

    Window::setSize({ 800, 600 });
    camera = std::make_shared<Entity>(scene->createEntity());
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();

    descriptor_set_layout = std::make_shared<DescriptorSetLayout>();
    descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
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
    storage_buffer = std::make_shared<StorageBuffer>(sizeof(glm::vec3));

    Vertex vertices[] = { 
    { {0.0f, 0.5f, 0.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, 0.0f} },
    { {0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
    { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} } };
    //indirect_buffer = std::make_shared<IndirectBuffer>(sizeof(VkDrawIndexedIndirectCommand));

    descriptor_set = std::make_shared<DescriptorSet>(*descriptor_set_layout, Graphics::swap_chain->getImages().size());
    descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
        .addImage(1, image.get())
        .addBuffer(2, { *storage_buffer, 0, VK_WHOLE_SIZE })
        .build();

    circles.resize(512);
    for (size_t i = 0; i < circles.size(); ++i)
    {
        circles[i] = std::make_shared<Entity>(scene->createEntity());
        circles[i]->addComponent<TransformComponent>(glm::translate(glm::mat4(1), glm::vec3(0, 0, 1)));
        circles[i]->addComponent<MeshComponent>(std::shared_ptr<CircleMesh>(new CircleMesh()));
        circles[i]->addComponent<RenderComponent>(descriptor_set_layout, descriptor_set, graphics_pipeline);
    }
    
    scene->onPlay();
}

void SandboxApp::onUpdate()
{
    Graphics::beginFrame();

    scene->onUpdate();

    glm::vec3 col = glm::vec3(1, 0, 0);
    storage_buffer->setData(&col);
    uniform_buffer->setData(&camera->getComponent<CameraComponent>().projection_view);

    //vkCmdDrawIndexedIndirect(Graphics::active.command_buffer, *indirect_buffer, 0, 1, sizeof(Vertex) * 3);

    Graphics::endFrame();
}

SandboxApp::~SandboxApp()
{
    Graphics::vulkanAssert(vkDeviceWaitIdle(*Graphics::logical_device));
    scene->onStop();
}

//CREATE APP
Application* createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
