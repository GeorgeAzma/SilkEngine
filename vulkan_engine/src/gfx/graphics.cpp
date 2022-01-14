#include "graphics.h"
#include "utils/debug_timer.h"
#include "core/event.h"
#include "enums.h"
#include "graphics_state.h"
#include "buffers/buffer_layout.h"
#include "core/time.h"


struct Transforms
{
	glm::mat4 projection_view;
};

void Graphics::init(GLFWwindow* window)
{
	VE_CORE_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");
	Graphics::window = window;

	//These most likely won't change
	instance = new Instance(); //70ms
	surface = new Surface(); //0.05ms
	physical_device = new PhysicalDevice(); //10ms
	logical_device = new LogicalDevice(); //80ms
	allocator = new Allocator();
	command_pool = new CommandPool(); //0.025ms

	descriptor_pool = new DescriptorPool();
	descriptor_pool->addSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
		.addSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		.setMaxSets(64).build();

	swap_chain = new SwapChain(); //16ms

	image = new Image("data/images/test.png");


	Shader shader = Shader
	({
		"data/cache/shaders/test.vert.spv",
		"data/cache/shaders/test.frag.spv" 
	}); //1.65ms

	descriptor_set_layout = new DescriptorSetLayout();
	descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
		.addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

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
		.setSampleCount(swap_chain->getSampleCount())
		.setRenderPass(swap_chain->getRenderPass())
		.build();

	vertex_buffer = new VertexBuffer(vertices.data(), vertices.size() * sizeof(vertices[0])); //2.4ms
	index_buffer = new IndexBuffer(indices.data(), indices.size() * sizeof(indices[0])); //0.9ms

	uniform_buffer = new UniformBuffer(sizeof(Transforms));

	descriptor_set = new DescriptorSet(*descriptor_set_layout, swap_chain->getImages().size());
	descriptor_set->addBuffer(0, { *uniform_buffer, 0, VK_WHOLE_SIZE })
	.addImage(1, image)
	.build();
}

void Graphics::beginFrame()
{
	swap_chain->beginFrame();
}

void Graphics::endFrame()
{
	swap_chain->endFrame();
}

void Graphics::update()
{	
	beginFrame();

	glm::mat4 projection = glm::perspective(glm::radians(80.0f), ((float)swap_chain->getExtent().width / swap_chain->getExtent().height), 0.01f, 1000.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 projection_view = glm::rotate(projection * view, (float)Time::runtime, { 0.0f, 0.0f, 1.0f });
	uniform_buffer->setData(&projection_view);

	endFrame();
}

void Graphics::cleanup() //25ms
{
	Graphics::vulkanAssert(vkDeviceWaitIdle(*logical_device));

	delete vertex_buffer;
	delete index_buffer;
	delete graphics_pipeline;
	delete swap_chain;
	delete uniform_buffer;
	delete descriptor_set_layout; //this generates an error for some reason
	delete descriptor_set;
	delete descriptor_pool;
	delete command_pool;
	delete image;
	delete allocator;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;

	glfwTerminate();
}

void Graphics::vulkanAssert(VkResult result)
{
	VE_CORE_ASSERT(result == VK_SUCCESS, std::string("Vulkan: ") + stringifyResult(result));
}

constexpr std::string Graphics::stringifyResult(VkResult result)
{
	switch (result) 
	{
	case VK_SUCCESS:
		return "Success";
	case VK_NOT_READY:
		return "A fence or query has not yet completed";
	case VK_TIMEOUT:
		return "A wait operation has not completed in the specified time";
	case VK_EVENT_SET:
		return "An event is signaled";
	case VK_EVENT_RESET:
		return "An event is unsignaled";
	case VK_INCOMPLETE:
		return "A return array was too small for the result";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "A host memory allocation has failed";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "A device memory allocation has failed";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "Initialization of an object could not be completed for implementation-specific reasons";
	case VK_ERROR_DEVICE_LOST:
		return "The logical or physical device has been lost";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "Mapping of a memory object has failed";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "A requested layer is not present or could not be loaded";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "A requested extension is not supported";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "A requested feature is not supported";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "Too many objects of the type have already been created";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "A requested format is not supported on this device";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "A surface is no longer available";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "A allocation failed due to having no more space in the descriptor pool";
	case VK_SUBOPTIMAL_KHR:
		return "A swapchain no longer matches the surface properties exactly, but can still be used";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "A surface has changed in such a way that it is no longer compatible with the swapchain";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "The display used by a swapchain does not use the same presentable image layout";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "A validation layer found an error";
	default:
		return "Unknown Vulkan error";
	}
}