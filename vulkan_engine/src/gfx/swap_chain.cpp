#include "swap_chain.h"
#include "physical_device.h"
#include "graphics.h"
#include "graphics_state.h"

SwapChain::SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	Dispatcher::subscribe(this, &SwapChain::onWindowResize);

	chooseSwapChainSurfaceFormat();
	chooseSwapChainPresentMode();
	
	image_count = Graphics::physical_device->getSurfaceCapabilities().minImageCount + 1;
	if (Graphics::physical_device->getSurfaceCapabilities().maxImageCount > 0)
		image_count = std::min(image_count, Graphics::physical_device->getSurfaceCapabilities().maxImageCount);

	depth_format = Graphics::physical_device->findDepthFormat();

	sample_count = Graphics::physical_device->getMaxSampleCount();

	render_pass = new RenderPass(); //0.11ms
	render_pass->beginSubpass()
		.addAttachment(0, surface_format.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, sample_count)
		.addAttachment(1, depth_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, sample_count)
		.addResolveAttachment(surface_format.format)
		.build();

	create(old_swap_chain);

	//Create semaphores and fences (0.064ms)
	images_in_flight.resize(images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		Graphics::vulkanAssert(vkCreateSemaphore(*Graphics::logical_device, &semaphore_info, nullptr, &image_available_semaphores[i]));
		Graphics::vulkanAssert(vkCreateSemaphore(*Graphics::logical_device, &semaphore_info, nullptr, &render_finished_semaphores[i]));

		Graphics::vulkanAssert(vkCreateFence(*Graphics::logical_device, &fence_info, nullptr, &in_flight_fences[i]));
	}

}

void SwapChain::recreate()
{
	Graphics::vulkanAssert(vkDeviceWaitIdle(*Graphics::logical_device));

	VkSwapchainKHR sc = swap_chain;
	create(sc);
	destroy();
}

void SwapChain::createFramebuffers()
{
	for (size_t i = 0; i < image_views.size(); ++i)
	{
		const std::vector<VkImageView> attachments = 
		{ 
			msaa_image->getDescriptorInfo().imageView, 
			depth->getDescriptorInfo().imageView,
		    *image_views[i] 
		};
	
		framebuffers[i] = new Framebuffer(*render_pass, attachments, extent.width, extent.height);
	}
}

void SwapChain::acquireNextImage()
{
	Graphics::vulkanAssert(vkWaitForFences(*Graphics::logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX));
	Graphics::vulkanAssert(vkAcquireNextImageKHR(*Graphics::logical_device, swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index));
}

void SwapChain::present()
{
	//Check if previous frame is using this image
	if (images_in_flight[image_index] != VK_NULL_HANDLE)
	{
		Graphics::vulkanAssert(vkWaitForFences(*Graphics::logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX));
	}
	images_in_flight[image_index] = in_flight_fences[current_frame];

	//Submit the command buffer
	const std::vector<VkSemaphore> signal_semaphores = { render_finished_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	command_buffer->submit(image_index, { image_available_semaphores[current_frame] }, signal_semaphores, wait_stages, &in_flight_fences[current_frame]);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = signal_semaphores.size();
	present_info.pWaitSemaphores = signal_semaphores.data();

	const std::vector<VkSwapchainKHR> swap_chains = { swap_chain };
	present_info.swapchainCount = swap_chains.size();
	present_info.pSwapchains = swap_chains.data();
	present_info.pImageIndices = &image_index;
	std::vector<VkResult> results(swap_chains.size());
	present_info.pResults = results.data();

	vkQueuePresentKHR(Graphics::logical_device->getPresentQueue(), &present_info);

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SwapChain::beginFrame()
{
	acquireNextImage(); 

	//RECORDING - TEMP
	const auto& command_buffers = command_buffer->getCommandBuffers();

	if (command_buffer->wasRecorded(image_index)) return;
	command_buffer->begin({}, image_index);
	render_pass->begin(*framebuffers[image_index]);

	Graphics::graphics_pipeline->bind();

	Graphics::vertex_buffer->bind();
	Graphics::index_buffer->bind();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(*graphics_state.command_buffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(*graphics_state.command_buffer, 0, 1, &scissor);

	Graphics::descriptor_set->bind(image_index);

	vkCmdDrawIndexed(*graphics_state.command_buffer, Graphics::indices.size(), 1, 0, 0, 0);

	render_pass->end();
	command_buffer->end(image_index);
}

void SwapChain::endFrame()
{
	present();
}

void SwapChain::create(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	Graphics::physical_device->getSwapChainSupportDetails();
	chooseSwapChainExtent();

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = *Graphics::surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1; //For stereoscopic 3D apps
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.preTransform = Graphics::physical_device->getSurfaceCapabilities().currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //For transparent windows
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	//create_info.oldSwapchain = old_swap_chain ? *old_swap_chain : VK_NULL_HANDLE; //Necessary for resizing and such

	const auto& indices = Graphics::physical_device->getQueueFamilyIndices();
	if (indices.graphics != indices.present) 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		auto queue_family_indices = indices.getIndices();
		create_info.queueFamilyIndexCount = queue_family_indices.size();
		create_info.pQueueFamilyIndices = queue_family_indices.data();
	}
	else 
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	Graphics::vulkanAssert(vkCreateSwapchainKHR(*Graphics::logical_device, &create_info, nullptr, &swap_chain));

	vkGetSwapchainImagesKHR(*Graphics::logical_device, swap_chain, &image_count, nullptr);
	images.resize(image_count);
	vkGetSwapchainImagesKHR(*Graphics::logical_device, swap_chain, &image_count, images.data());
	
	image_views.resize(images.size());
	framebuffers.resize(image_views.size());

	for (size_t i = 0; i < image_views.size(); ++i)
	{
		image_views[i] = new ImageView(images[i], surface_format.format);
	}

	///////////////////

	VkFormat depth_format = Graphics::physical_device->findDepthFormat();
	ImageProps props{};
	props.width = extent.width;
	props.height = extent.height;
	props.format = depth_format;
	props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	props.create_sampler = false;
	props.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	props.mipmap = false;
	props.samples = Graphics::physical_device->getMaxSampleCount();
	depth = new Image(props);

	ImageProps msaa_props{};
	props.width = extent.width;
	props.height = extent.height;
	props.format = surface_format.format;
	props.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
		| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	props.create_sampler = false;
	props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	props.mipmap = false;
	props.samples = sample_count;
	msaa_image = new Image(props);

	createFramebuffers();
	command_buffer = new CommandBuffer(framebuffers.size());
}

void SwapChain::destroy()
{
	delete command_buffer;
	delete depth;
	delete msaa_image;
	for (auto& framebuffer : framebuffers)
	{
		delete framebuffer;
		framebuffer = nullptr;
	}
	for (auto& image_view : image_views)
	{
		delete image_view;
	}
	vkDestroySwapchainKHR(*Graphics::logical_device, swap_chain, nullptr);
}

SwapChain::~SwapChain()
{
	Dispatcher::unsubscribe(this, &SwapChain::onWindowResize);

	delete render_pass;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(*Graphics::logical_device, render_finished_semaphores[i], nullptr);
		vkDestroySemaphore(*Graphics::logical_device, image_available_semaphores[i], nullptr);
		vkDestroyFence(*Graphics::logical_device, in_flight_fences[i], nullptr);
	}

	destroy();
}



void SwapChain::chooseSwapChainSurfaceFormat()
{
	std::multimap<int, VkSurfaceFormatKHR> formats;

	for (const auto& available_format : Graphics::physical_device->getSurfaceFormats())
	{
		int score = rateSwapChainSurfaceFormat(available_format);
		if (score >= 0)
		{
			formats.insert(std::make_pair(score, available_format));
		}
	}

	VE_CORE_ASSERT(formats.rbegin()->first >= 0, 
		"Vulkan: Couldn't find supported formats to choose from");
	
	this->surface_format = formats.rbegin()->second;
}

int SwapChain::rateSwapChainSurfaceFormat(VkSurfaceFormatKHR format) const
{
	int score = 0;

	score += (format.format == VK_FORMAT_B8G8R8A8_SRGB) * 1000;
	score += (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) * 400;

	return score;
}

void SwapChain::onWindowResize(const WindowResizeEvent& e)
{
	if (e.window == Graphics::window)
	{
		recreate();
	}
}

void SwapChain::chooseSwapChainPresentMode()
{
	std::multimap<int, VkPresentModeKHR> present_modes;

	for (const auto& available_present_mode : Graphics::physical_device->getPresentModes())
	{
		int score = rateSwapChainPresentMode(available_present_mode);
		if (score >= 0)
		{
			present_modes.insert(std::make_pair(score, available_present_mode));
		}
	}

	VE_CORE_ASSERT(present_modes.rbegin()->first >= 0,
		"Vulkan: Couldn't find supported present modes to choose from");

	this->present_mode = present_modes.rbegin()->second;
}

void SwapChain::chooseSwapChainExtent()
{
	if (Graphics::physical_device->getSurfaceCapabilities().currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		this->extent = Graphics::physical_device->getSurfaceCapabilities().currentExtent;
		return;
	}

	int width, height;
	glfwGetFramebufferSize(Graphics::window, &width, &height);
	VkExtent2D extent =
	{
		(uint32_t)width,
		(uint32_t)height
	};

	extent.width = std::clamp(extent.width, 
		Graphics::physical_device->getSurfaceCapabilities().minImageExtent.width,
		Graphics::physical_device->getSurfaceCapabilities().maxImageExtent.width);
	extent.height = std::clamp(extent.height,
		Graphics::physical_device->getSurfaceCapabilities().minImageExtent.height,
		Graphics::physical_device->getSurfaceCapabilities().maxImageExtent.height);

	this->extent = extent;
}

int SwapChain::rateSwapChainPresentMode(VkPresentModeKHR present_mode) const
{
	switch (present_mode)
	{
	//Less power consumption, more latency, guaranteed supportability
	case VK_PRESENT_MODE_FIFO_KHR:
		return 1000;
		break;
	//More power consumption, less latency
	case VK_PRESENT_MODE_MAILBOX_KHR:
		return 1500; 
		break;
	//Less power consumption, less latency, more tearing
	case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
		return 500;
		break;
	//Less power consumption, less latency, much more tearing
	case VK_PRESENT_MODE_IMMEDIATE_KHR:
		return 250;
		break;
	}

	return -1;
}
