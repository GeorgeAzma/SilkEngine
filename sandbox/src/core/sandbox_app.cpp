#include "sandbox_app.h"


std::vector<glm::vec3> offsets;
SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{ 
    for (size_t i = 0; i < 1000000; ++i)
        offsets.emplace_back(RNG::Float() * 16, RNG::Float() * 16, RNG::Float() * 16);
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
        .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 }, { Type::VEC3, 1 } })
        .setSampleCount(Graphics::swap_chain->getSampleCount())
        .setRenderPass(Graphics::swap_chain->getRenderPass())
        .build();

    vertex_buffer = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(vertices[0])); //2.4ms
    vertex_buffer2 = std::make_shared<VertexBuffer>(offsets.data(), offsets.size() * sizeof(offsets[0])); //2.4ms
    index_buffer = std::make_shared<IndexBuffer>(indices.data(), indices.size() * sizeof(indices[0])); //0.9ms
    vertex_array = std::make_shared<VertexArray>();
    vertex_array->setIndexBuffer(index_buffer)
        .addVertexBuffer(vertex_buffer)
        .addVertexBuffer(vertex_buffer2);

    uniform_buffer = std::make_shared<UniformBuffer>(sizeof(Transforms));

    descriptor_set = std::make_shared<DescriptorSet>(*descriptor_set_layout, Graphics::swap_chain->getImages().size());
    descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
        .addImage(1, image.get())
        .build();
}

void SandboxApp::onUpdate()
{
    scene.onUpdate();

    Graphics::beginFrame();
    graphics_pipeline->bind();
    
    vertex_array->bind();

    descriptor_set->bind(Graphics::swap_chain->getImageIndex());
    
    vkCmdDrawIndexed(Graphics::active.command_buffer, indices.size(), offsets.size(), 0, 0, 0);

    uniform_buffer->setData(&camera->getComponent<CameraComponent>().projection_view);

    Graphics::endFrame();
}

SandboxApp::~SandboxApp()
{
    Graphics::vulkanAssert(vkDeviceWaitIdle(*Graphics::logical_device));
    scene.onStop();
}

//CREATE APP
Application *createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
