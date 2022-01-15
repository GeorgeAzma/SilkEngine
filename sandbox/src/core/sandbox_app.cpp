#include "sandbox_app.h"

static inline Scene scene;
static inline std::shared_ptr<Entity> camera;
static inline UniformBuffer* uniform_buffer = nullptr;
static inline VertexBuffer* vertex_buffer = nullptr;
static inline IndexBuffer* index_buffer = nullptr;
static inline Image* image = nullptr;
static inline DescriptorSetLayout* descriptor_set_layout = nullptr;
static inline DescriptorSet* descriptor_set = nullptr;
static inline GraphicsPipeline* graphics_pipeline = nullptr;
static inline const std::vector<Vertex> vertices =
{
	{{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
};
static inline const std::vector<uint32_t> indices =
{
	0, 1, 2, 2, 3, 0
};

struct Transforms
{
    glm::mat4 projection_view;
};

SandboxApp::SandboxApp(ApplicationCommandLineArgs args)
{ 
    Window::setSize({ 800, 600 });
    camera = std::make_shared<Entity>(scene.createEntity());
    camera->addComponent<TransformComponent>();
    camera->addComponent<CameraComponent>();
    camera->addComponent<ScriptComponent>().bind<CameraController>();
    scene.onPlay();

    descriptor_set_layout = new DescriptorSetLayout();
    descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    Shader shader = Shader
    ({
        "data/cache/shaders/test.vert.spv",
        "data/cache/shaders/test.frag.spv"
    }); //1.65ms
    image = new Image("data/images/test.png");

    GraphicsPipelineProps graphics_pipeline_props{};
    graphics_pipeline = new GraphicsPipeline(); //1.35ms
    graphics_pipeline->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
        .addDynamicState(VK_DYNAMIC_STATE_SCISSOR)
        .enable(EnableTag::COLOR_BLENDING)
        .enable(EnableTag::DEPTH_TEST)
        .enable(EnableTag::DEPTH_WRITE)
        .addDescriptorSetLayout(*descriptor_set_layout)
        .setShader(shader)
        .setVertexLayout({ { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 } })
        .setSampleCount(Graphics::swap_chain->getSampleCount())
        .setRenderPass(Graphics::swap_chain->getRenderPass())
        .build();

    vertex_buffer = new VertexBuffer(vertices.data(), vertices.size() * sizeof(vertices[0])); //2.4ms
    index_buffer = new IndexBuffer(indices.data(), indices.size() * sizeof(indices[0])); //0.9ms

    uniform_buffer = new UniformBuffer(sizeof(Transforms));

    descriptor_set = new DescriptorSet(*descriptor_set_layout, Graphics::swap_chain->getImages().size());
    descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
        .addImage(1, image)
        .build();
}

void SandboxApp::onUpdate()
{
    scene.onUpdate();

    Graphics::beginFrame(); 

    graphics_pipeline->bind();

    //VkViewport viewport{};
    //viewport.x = 0.0f;
    //viewport.y = 0.0f;
    //viewport.width = Graphics::swap_chain->getExtent().width;
    //viewport.height = Graphics::swap_chain->getExtent().height;
    //viewport.minDepth = 0.0f;
    //viewport.maxDepth = 1.0f;
    //vkCmdSetViewport(Graphics::active.command_buffer, 0, 1, &viewport);
    //
    //VkRect2D scissor{};
    //scissor.offset = { 0, 0 };
    //scissor.extent = Graphics::swap_chain->getExtent();
    //vkCmdSetScissor(Graphics::active.command_buffer, 0, 1, &scissor);
    //
    //vertex_buffer->bind();
    //index_buffer->bind();
    //
    //descriptor_set->bind(Graphics::swap_chain->getImageIndex());
    //
    //vkCmdDrawIndexed(Graphics::active.command_buffer, indices.size(), 1, 0, 0, 0);
    //
    //uniform_buffer->setData(&camera->getComponent<CameraComponent>().projection_view);

    Graphics::endFrame();
}

SandboxApp::~SandboxApp()
{
    Graphics::vulkanAssert(vkDeviceWaitIdle(*Graphics::logical_device));

    scene.onStop();

    delete vertex_buffer;
    delete index_buffer;
    delete uniform_buffer;
    delete image;
    delete descriptor_set;
    delete descriptor_set_layout;
    delete graphics_pipeline;
}

//CREATE APP
Application *createApp(ApplicationCommandLineArgs args)
{
    return new SandboxApp(args);
}
