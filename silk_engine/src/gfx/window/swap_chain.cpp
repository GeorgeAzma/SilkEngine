#include "swap_chain.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/devices/logical_device.h"

SwapChain::SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	chooseSwapChainSurfaceFormat();
	chooseSwapChainPresentMode();

	depth_format = Graphics::physical_device->findDepthFormat();

	sample_count = Graphics::physical_device->getMaxSampleCount();

	render_pass = makeShared<RenderPass>();
	render_pass->addSubpass()
		.addAttachment(surface_format.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, sample_count)
		.addAttachment(depth_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, sample_count)
		.addAttachment(surface_format.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		.build();

	create(old_swap_chain);

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

	VkSwapchainKHR old_swapchain = swap_chain;
	create(old_swapchain);
	vkDestroySwapchainKHR(*Graphics::logical_device, old_swapchain, nullptr);
}

void SwapChain::createFramebuffers()
{
	Image2DProps props{};
	props.width = extent.width;
	props.height = extent.height;
	props.format = Graphics::physical_device->findDepthFormat();
	props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	props.create_sampler = false;
	props.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	props.mipmap = false;
	props.samples = sample_count;
	depth = makeShared<Image2D>(props);

	props.format = surface_format.format;
	props.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	msaa_image = makeShared<Image2D>(props);

	for (size_t i = 0; i < images.size(); ++i)
	{
		framebuffers[i] = makeShared<Framebuffer>(*render_pass, std::vector<shared<Image2D>>{ msaa_image, depth, images[i] }, extent.width, extent.height);
	}
}

void SwapChain::acquireNextImage()
{
	vkWaitForFences(*Graphics::logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
	Graphics::vulkanAssert(vkAcquireNextImageKHR(*Graphics::logical_device, swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index));
}

void SwapChain::present()
{
	//Check if previous frame is using this image
	if (images_in_flight[image_index] != VK_NULL_HANDLE)
		Graphics::vulkanAssert(vkWaitForFences(*Graphics::logical_device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX));
	images_in_flight[image_index] = in_flight_fences[current_frame];

	//Submit the command buffer
	const std::vector<VkSemaphore> signal_semaphores = { render_finished_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	CommandBufferSubmitInfo submit_info{};
	submit_info.fence = in_flight_fences[current_frame];
	submit_info.signal_semaphores = signal_semaphores;
	submit_info.wait_semaphores = { image_available_semaphores[current_frame] };
	submit_info.wait_stages = wait_stages;
	command_buffers[image_index]->submit(submit_info);

	//Present
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = signal_semaphores.size();
	present_info.pWaitSemaphores = signal_semaphores.data();
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swap_chain;
	present_info.pImageIndices = &image_index;

	vkQueuePresentKHR(Graphics::logical_device->getPresentQueue(), &present_info);

	current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SwapChain::beginFrame(size_t i)
{
	command_buffers[i]->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void SwapChain::beginRenderPass(size_t i)
{
	render_pass->begin(*framebuffers[i]);
}

void SwapChain::endFrame(size_t i)
{
	command_buffers[i]->end();
}

void SwapChain::endRenderPass()
{
	render_pass->end();
}

void SwapChain::create(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	chooseSwapChainExtent();

	uint32_t image_count = Graphics::physical_device->getSurfaceCapabilities().minImageCount + 1;
	if (Graphics::physical_device->getSurfaceCapabilities().maxImageCount > 0)
		image_count = std::min(image_count, Graphics::physical_device->getSurfaceCapabilities().maxImageCount);

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
	create_info.oldSwapchain = old_swap_chain ? *old_swap_chain : VK_NULL_HANDLE; //Necessary for resizing and such

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
	std::vector<VkImage> images(image_count);
	this->images.resize(image_count);
	vkGetSwapchainImagesKHR(*Graphics::logical_device, swap_chain, &image_count, images.data());

	for (size_t i = 0; i < images.size(); ++i)
	{
		Image2DProps props{};
		props.create_sampler = false;
		props.mipmap = false;
		props.format = surface_format.format;
		props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		this->images[i] = makeShared<Image2D>(images[i], props);
	}

	framebuffers.resize(images.size());

	createFramebuffers();

	command_buffers.resize(images.size());
	for (auto& command_buffer : command_buffers)
		command_buffer = makeShared<CommandBuffer>();
}

void SwapChain::destroy()
{
	vkDestroySwapchainKHR(*Graphics::logical_device, swap_chain, nullptr);
}

SwapChain::~SwapChain()
{
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
			formats.insert(std::make_pair(score, available_format));
	}

	SK_ASSERT(formats.rbegin()->first >= 0,
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

void SwapChain::chooseSwapChainPresentMode()
{
	std::map<int, VkPresentModeKHR> present_modes;

	for (const auto& available_present_mode : Graphics::physical_device->getPresentModes())
	{
		int score = 0;
		switch (available_present_mode)
		{
		case VK_PRESENT_MODE_MAILBOX_KHR:
			score = 4;
			break;
		case VK_PRESENT_MODE_FIFO_KHR:
			score = 3;
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			score = 2;
			break;
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			score = 1;
			break;
		}
		if (score >= 0)
		{
			present_modes.insert(std::make_pair(score, available_present_mode));
		}
	}

	SK_ASSERT(present_modes.rbegin()->first >= 0,
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
	glfwGetFramebufferSize(Window::getGLFWWindow(), &width, &height);
	this->extent =
	{
		std::clamp((uint32_t)width,
			Graphics::physical_device->getSurfaceCapabilities().minImageExtent.width,
			Graphics::physical_device->getSurfaceCapabilities().maxImageExtent.width),
		std::clamp((uint32_t)height,
			Graphics::physical_device->getSurfaceCapabilities().minImageExtent.height,
			Graphics::physical_device->getSurfaceCapabilities().maxImageExtent.height)
	};
}