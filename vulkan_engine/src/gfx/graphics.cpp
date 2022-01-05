#include "graphics.h"

void Graphics::init(GLFWwindow* window)
{
	VE_CORE_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");
	
	instance = new Instance();
	surface = new Surface(window);
	physical_device = new PhysicalDevice();
	logical_device = new LogicalDevice();
	command_pool = new CommandPool();
	swap_chain = new SwapChain(window);
	render_pass = new RenderPass();
	swap_chain->createFramebuffers();
	graphics_pipeline = new GraphicsPipeline(Shader({ 
		"E:/Codes/C++/VulkanEngine/data/cache/shaders/test.vert.spv", 
		"E:/Codes/C++/VulkanEngine/data/cache/shaders/test.frag.spv" }));
	
	
	
	//Create command buffers
	command_buffers.resize(swap_chain->getFramebuffers().size());

	//Allocate command buffers
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = *Graphics::command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = command_buffers.size();

	Graphics::vulkanAssert(vkAllocateCommandBuffers(*Graphics::logical_device, &alloc_info, command_buffers.data()));

	//Record command buffers
	for (size_t i = 0; i < command_buffers.size(); i++)
	{
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		Graphics::vulkanAssert(vkBeginCommandBuffer(command_buffers[i], &begin_info));

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = *Graphics::render_pass;
		render_pass_begin_info.framebuffer = *swap_chain->getFramebuffers()[i];

		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = swap_chain->getExtent(); //TODO: I think I can set this to the maximum size from framebuffers[i].attachments[j].size;

		VkClearValue clear_value = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear_value;

		vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *Graphics::graphics_pipeline);

		vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(command_buffers[i]);

		Graphics::vulkanAssert(vkEndCommandBuffer(command_buffers[i]));
	}



	//Create semaphores
	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	Graphics::vulkanAssert(vkCreateSemaphore(*logical_device, &semaphore_info, nullptr, &image_available_semaphore));
	Graphics::vulkanAssert(vkCreateSemaphore(*logical_device, &semaphore_info, nullptr, &render_finished_semaphore));
}

void Graphics::update()
{
	//Get next swap chain image
	uint32_t image_index;
	vkAcquireNextImageKHR(*logical_device, *swap_chain, UINT64_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);

	//Submit the command buffer
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	const std::vector<VkSemaphore> wait_semaphores = { image_available_semaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = wait_semaphores.size();
	submit_info.pWaitSemaphores = wait_semaphores.data();
	submit_info.pWaitDstStageMask = waitStages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[image_index];

	const std::vector<VkSemaphore> signal_semaphores = { render_finished_semaphore };
	submit_info.signalSemaphoreCount = signal_semaphores.size();
	submit_info.pSignalSemaphores = signal_semaphores.data();

	Graphics::vulkanAssert(vkQueueSubmit(logical_device->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));

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


}

void Graphics::cleanup()
{
	vkDeviceWaitIdle(*logical_device);

	vkDestroySemaphore(*logical_device, render_finished_semaphore, nullptr);
	vkDestroySemaphore(*logical_device, image_available_semaphore, nullptr);
	delete graphics_pipeline;
	delete render_pass;
	delete swap_chain;
	delete command_pool;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;
}

constexpr void Graphics::vulkanAssert(VkResult result)
{
	VE_CORE_ASSERT(result == VK_SUCCESS, stringifyResult(result));
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