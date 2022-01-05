#include "swap_chain.h"
#include "physical_device.h"
#include "graphics.h"

SwapChain::SwapChain(GLFWwindow* window)
	: window{window}
{
	support_details = getSwapChainSupportDetails(Graphics::getPhysicalDevice()->getPhysicalDevice(), Graphics::getSurface()->getSurface());

	chooseSwapChainSurfaceFormat();
	chooseSwapChainPresentMode();
	chooseSwapChainExtent();
	uint32_t image_count = support_details.capabilities.minImageCount + 1;
	if (support_details.capabilities.maxImageCount > 0 && 
		image_count > support_details.capabilities.maxImageCount)
	{
		image_count = support_details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = Graphics::getSurface()->getSurface();
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1; //For stereoscopic 3D apps
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.preTransform = support_details.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //For transparent windows
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE; //Necessary for resizing and such

	auto indices = Graphics::getPhysicalDevice()->getQueueFamilyIndices();
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

	VE_CORE_ASSERT(vkCreateSwapchainKHR(Graphics::getLogicalDevice()->getLogicalDevice(), &create_info, nullptr, &swap_chain) == VK_SUCCESS,
		"Vulkan: Couldn't create swap chain");

	vkGetSwapchainImagesKHR(Graphics::getLogicalDevice()->getLogicalDevice(), swap_chain, &image_count, nullptr);
	images.resize(image_count);
	vkGetSwapchainImagesKHR(Graphics::getLogicalDevice()->getLogicalDevice(), swap_chain, &image_count, images.data());
	
	image_views.resize(images.size());
	for (size_t i = 0; i < image_views.size(); ++i)
	{
		VkImageViewCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = surface_format.format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;
		VE_CORE_ASSERT(vkCreateImageView(Graphics::getLogicalDevice()->getLogicalDevice(), &create_info, nullptr, &image_views[i]) == VK_SUCCESS,
			"Vulkan: Couldn't create image view");
	}
}

SwapChain::~SwapChain()
{
	for (const auto& image_view : image_views) 
	{
		vkDestroyImageView(Graphics::getLogicalDevice()->getLogicalDevice(), image_view, nullptr);
	}
	vkDestroySwapchainKHR(Graphics::getLogicalDevice()->getLogicalDevice(), swap_chain, nullptr);
}

SwapChainSupportDetails SwapChain::getSwapChainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details{};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

	if (format_count) 
	{
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
	}

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

	if (present_mode_count)
	{
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes.data());
	}

	return details;
}

void SwapChain::chooseSwapChainSurfaceFormat()
{
	std::multimap<int, VkSurfaceFormatKHR> formats;

	for (const auto& available_format : support_details.formats)
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

	for (const auto& available_present_mode : support_details.present_modes)
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
	if (support_details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		this->extent = support_details.capabilities.currentExtent;
		return;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	VkExtent2D extent =
	{
		width,
		height
	};

	extent.width = std::clamp(extent.width, 
		support_details.capabilities.minImageExtent.width, 
		support_details.capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height,
		support_details.capabilities.minImageExtent.height,
		support_details.capabilities.maxImageExtent.height);

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
}
