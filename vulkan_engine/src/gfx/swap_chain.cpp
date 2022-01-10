#include "swap_chain.h"
#include "physical_device.h"
#include "graphics.h"

SwapChain::SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	Graphics::surface->getSupportDetails(*Graphics::physical_device);

	chooseSwapChainSurfaceFormat();
	chooseSwapChainPresentMode();
	chooseSwapChainExtent();

	uint32_t image_count = Graphics::surface->getCapabilities().minImageCount + 1;
	if (Graphics::surface->getCapabilities().maxImageCount > 0 &&
		image_count > Graphics::surface->getCapabilities().maxImageCount)
	{
		image_count = Graphics::surface->getCapabilities().maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = *Graphics::surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1; //For stereoscopic 3D apps
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.preTransform = Graphics::surface->getCapabilities().currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //For transparent windows
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = old_swap_chain ? *old_swap_chain : VK_NULL_HANDLE; //Necessary for resizing and such

	auto indices = Graphics::physical_device->getQueueFamilyIndices();
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
}

void SwapChain::createFramebuffers(std::vector<VkImageView> image_views)
{
	if (Graphics::render_pass != nullptr)
	{
		for (size_t i = 0; i < this->image_views.size(); ++i)
		{
			if (framebuffers[i]) 
				continue;

			std::vector<VkImageView> attachments;

			attachments.insert(attachments.end(), image_views.begin(), image_views.end());
			attachments.emplace_back(*this->image_views[i]);

			framebuffers[i] = new Framebuffer(*Graphics::render_pass, attachments);
		}
	}
}

SwapChain::~SwapChain()
{
	for (auto& framebuffer : framebuffers)
	{
		delete framebuffer;
	}
	for (auto& image_view : image_views) 
	{
		delete image_view;
	}
	vkDestroySwapchainKHR(*Graphics::logical_device, swap_chain, nullptr);
}

void SwapChain::chooseSwapChainSurfaceFormat()
{
	std::multimap<int, VkSurfaceFormatKHR> formats;

	for (const auto& available_format : Graphics::surface->getFormats())
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

void SwapChain::chooseSwapChainPresentMode()
{
	std::multimap<int, VkPresentModeKHR> present_modes;

	for (const auto& available_present_mode : Graphics::surface->getPresentModes())
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
	if (Graphics::surface->getCapabilities().currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		this->extent = Graphics::surface->getCapabilities().currentExtent;
		return;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	VkExtent2D extent =
	{
		(uint32_t)width,
		(uint32_t)height
	};

	extent.width = std::clamp(extent.width, 
		Graphics::surface->getCapabilities().minImageExtent.width,
		Graphics::surface->getCapabilities().maxImageExtent.width);
	extent.height = std::clamp(extent.height,
		Graphics::surface->getCapabilities().minImageExtent.height,
		Graphics::surface->getCapabilities().maxImageExtent.height);

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
