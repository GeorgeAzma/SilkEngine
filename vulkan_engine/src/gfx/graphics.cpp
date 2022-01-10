#include "graphics.h"
#include "utils/debug_timer.h"
#include "core/event.h"
#include "scene/vertex.h"
#include "enums.h"
#include "graphics_state.h"
#include "buffers/buffer_layout.h"

static const std::vector<Vertex> vertices =
{
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
};

static const std::vector<uint32_t> indices =
{
	0, 1, 2, 2, 3, 0
};

struct ColorUniformBuffer
{
	glm::vec4 color = glm::vec4(0, 0, 0, 1);
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
	command_pool = new CommandPool(); //0.025ms
	swap_chain = new SwapChain(); //16ms

	VkFormat depth_format = physical_device->findDepthFormat();
	ImageProps props{};
	props.width = swap_chain->getExtent().width;
	props.height = swap_chain->getExtent().height;
	props.format = depth_format;
	props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	props.create_sampler = false;
	props.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	props.mipmap = false;
	//props.samples = physical_device->getMaxSampleCount();
	depth = new Image(props);

	image = new Image("data/images/test.png");

	ImageProps msaa_props{};
	props.width = swap_chain->getExtent().width;
	props.height = swap_chain->getExtent().height;
	props.format = swap_chain->getSurfaceFormat().format;
	props.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT 
		| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	props.create_sampler = false;
	props.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //TODO
	props.mipmap = false;
	props.samples = physical_device->getMaxSampleCount();
	//msaa_image = new Image(props);

	render_pass = new RenderPass(); //0.11ms
	render_pass->beginSubpass()
		.addAttachment(0, swap_chain->getSurfaceFormat().format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.addAttachment(1, physical_device->findDepthFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		.build();


	swap_chain->createFramebuffers(depth->getDescriptorInfo().imageView); //0.036ms

	Shader shader = Shader({
		"data/cache/shaders/test.vert.spv",
		"data/cache/shaders/test.frag.spv" }); //1.65ms

	descriptor_set_layout = new DescriptorSetLayout();
	descriptor_set_layout->addBinding(0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(1, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	GraphicsPipelineProps graphics_pipeline_props{};
	graphics_pipeline = new GraphicsPipeline({ &shader, { { Type::VEC3 }, { Type::VEC2 }, { Type::VEC3 } } }); //1.35ms

	vertex_buffer = new VertexBuffer(vertices.data(), vertices.size() * sizeof(vertices[0])); //2.4ms
	index_buffer = new IndexBuffer(indices.data(), indices.size() * sizeof(indices[0])); //0.9ms

	for (size_t i = 0; i < swap_chain->getImages().size(); ++i)
	{
		uniform_buffers.emplace_back(std::make_shared<UniformBuffer>(sizeof(ColorUniformBuffer)));

		glm::vec4 color = glm::vec4(0, 1, 1, 1);
		uniform_buffers[i]->setData(&color);
	}

	descriptor_pool = new DescriptorPool();
	descriptor_pool->addSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain->getImages().size())
		.addSize(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swap_chain->getImages().size())
		.setMaxSets(swap_chain->getImages().size() * 2).build();

	descriptor_set = new DescriptorSet(*descriptor_set_layout, swap_chain->getImages().size());
	for(size_t i = 0; i < uniform_buffers.size(); ++i)
		descriptor_set->addBuffer(0, { *uniform_buffers[i], 0, VK_WHOLE_SIZE });
	descriptor_set->addImage(1, image)
		.build();

	//Staticly recorded command buffer
	command_buffer = new CommandBuffer(swap_chain->getFramebuffers().size());//0.025ms
	recordCommandBuffers(); //0.211ms

	//Create semaphores and fences (0.064ms)
	images_in_flight.resize(swap_chain->getImages().size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		Graphics::vulkanAssert(vkCreateSemaphore(*logical_device, &semaphore_info, nullptr, &image_available_semaphores[i]));
		Graphics::vulkanAssert(vkCreateSemaphore(*logical_device, &semaphore_info, nullptr, &render_finished_semaphores[i]));

		Graphics::vulkanAssert(vkCreateFence(*logical_device, &fence_info, nullptr, &in_flight_fences[i]));
	}
}

void Graphics::recordCommandBuffers()
{
	const auto& command_buffers = command_buffer->getCommandBuffers();

	//Record command buffers
	for (size_t i = 0; i < command_buffers.size(); i++)
	{
		command_buffer->begin({}, i);
		render_pass->begin(*Graphics::swap_chain->getFramebuffers()[i]);

		Graphics::graphics_pipeline->bind();

		vertex_buffer->bind();
		index_buffer->bind();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = Graphics::swap_chain->getExtent().width;
		viewport.height = Graphics::swap_chain->getExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(*graphics_state.command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = Graphics::swap_chain->getExtent();
		vkCmdSetScissor(*graphics_state.command_buffer, 0, 1, &scissor);

		descriptor_set->bind(i);

		vkCmdDrawIndexed(*graphics_state.command_buffer, indices.size(), 1, 0, 0, 0);

		render_pass->end();
		command_buffer->end(i);
	}
}

void Graphics::recreateSwapChain() //7.5ms
{
	Graphics::vulkanAssert(vkDeviceWaitIdle(*logical_device));

	VkSwapchainKHR sc = *swap_chain;
	SwapChain* old_swap_chain = swap_chain;
	swap_chain = new SwapChain(sc);

	size_t command_buffers_size = command_buffer->getCommandBuffers().size();
	delete command_buffer;

	ImageProps depth_props = depth->getProps();
	depth_props.width = swap_chain->getExtent().width;
	depth_props.height = swap_chain->getExtent().height;
	delete depth;
	depth = new Image(depth_props);

	swap_chain->createFramebuffers(depth->getDescriptorInfo().imageView);

	command_buffer = new CommandBuffer(command_buffers_size);
	recordCommandBuffers();

	delete old_swap_chain;
}

void Graphics::update()
{	
	Graphics::vulkanAssert(vkWaitForFences(*logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX));

	//Get next swap chain image
	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(*logical_device, *swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	if (result != VK_SUBOPTIMAL_KHR)
		Graphics::vulkanAssert(result);

	//Check if previous frame is using this image
	if (images_in_flight[image_index] != VK_NULL_HANDLE) 
	{
		Graphics::vulkanAssert(vkWaitForFences(*logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX));
	}

	//Submit the command buffer
	const std::vector<VkSemaphore> signal_semaphores = { render_finished_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	command_buffer->submit(image_index, { image_available_semaphores[current_frame] }, signal_semaphores, wait_stages, &in_flight_fences[current_frame]);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = signal_semaphores.size();
	present_info.pWaitSemaphores = signal_semaphores.data();

	const std::vector<VkSwapchainKHR> swap_chains = { *swap_chain };
	present_info.swapchainCount = swap_chains.size();
	present_info.pSwapchains = swap_chains.data();
	present_info.pImageIndices = &image_index;
	std::vector<VkResult> results(swap_chains.size());
	present_info.pResults = results.data();

	vkQueuePresentKHR(logical_device->getPresentQueue(), &present_info);
	
	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Graphics::cleanup() //25ms
{
	Graphics::vulkanAssert(vkDeviceWaitIdle(*logical_device));

	delete vertex_buffer;
	delete index_buffer;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(*logical_device, render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(*logical_device, image_available_semaphores[i], nullptr);
		vkDestroyFence(*logical_device, in_flight_fences[i], nullptr);
	}
	delete graphics_pipeline;
	delete render_pass;
	delete swap_chain;
	uniform_buffers.clear();
	delete descriptor_set_layout; //this generates an error for some reason
	delete descriptor_set;
	delete descriptor_pool;
	delete command_pool;
	delete image;
	delete msaa_image;
	delete depth;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;

	glfwTerminate();
}

void Graphics::onWindowResize(const WindowResizeEvent& e)
{
	if (e.window == window)
	{
		framebuffer_resized = true;
	}
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