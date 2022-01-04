#include "swap_chain.h"
#include "physical_device.h"

SwapChain::SwapChain(const VkPhysicalDevice* physical_device, const VkSurfaceKHR* surface, GLFWwindow* window, const VkDevice* logical_device)
	: physical_device{physical_device}, surface{surface}, window{window}, logical_device{logical_device}
{
	support_details = getSwapChainSupportDetails(*physical_device, *surface);

	VkSurfaceFormatKHR surface_format = chooseSwapChainSurfaceFormat();
	VkPresentModeKHR present_mode = chooseSwapChainPresentMode();
	VkExtent2D extent = chooseSwapChainExtent();
	uint32_t image_count = support_details.capabilities.minImageCount + 1;
	if (support_details.capabilities.maxImageCount > 0 && 
		image_count > support_details.capabilities.maxImageCount)
	{
		image_count = support_details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = *surface;
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

	QueueFamilyIndices indices = PhysicalDevice::findQueueFamilies(*physical_device, *surface);
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

	VE_CORE_ASSERT(vkCreateSwapchainKHR(*logical_device, &create_info, nullptr, &swap_chain) == VK_SUCCESS,
		"Vulkan: Couldn't create swap chain");
}

SwapChain::~SwapChain()
{
	vkDestroySwapchainKHR(*logical_device, swap_chain, nullptr);
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

VkSurfaceFormatKHR SwapChain::chooseSwapChainSurfaceFormat() const
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
	
	return formats.rbegin()->second;
}

int SwapChain::rateSwapChainSurfaceFormat(VkSurfaceFormatKHR format) const
{
	int score = 0;

	score += (format.format == VK_FORMAT_B8G8R8A8_SRGB) * 1000;
	score += (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) * 400;

	return score;
}

VkPresentModeKHR SwapChain::chooseSwapChainPresentMode() const
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

	return present_modes.rbegin()->second;
}

VkExtent2D SwapChain::chooseSwapChainExtent() const
{
	if (support_details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return support_details.capabilities.currentExtent;
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

	return extent;
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
