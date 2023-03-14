#include "swap_chain.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/devices/logical_device.h"
#include "gfx/queues/queue.h"
#include <GLFW/glfw3.h>

SwapChain::SwapChain(const Surface& surface, VkSwapchainKHR old_swap_chain)
	: surface(surface)
{
	//Choose format
	std::multimap<int, VkSurfaceFormatKHR> surface_formats;
	for (const auto& available_format : surface.getFormats())
	{
		int score = (available_format.format == VK_FORMAT_B8G8R8A8_UNORM) * 1000 +
			(available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) * 400;
		if (score >= 0)
			surface_formats.insert(std::make_pair(score, available_format));
	}
	SK_ASSERT(surface_formats.size() > 0, "Vulkan: Couldn't find supported formats to choose from");

	//Choose present mode
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& available_present_mode : surface.getPresentModes())
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			present_mode = available_present_mode;
			break;
		}
	}
	this->present_mode = present_mode;

	image_format = Image::Format(surface_formats.rbegin()->second.format);
	depth_format = Image::Format(Graphics::physical_device->findSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT ));
	sample_count = Graphics::physical_device->getMaxSampleCount();
	color_space = surface_formats.rbegin()->second.colorSpace;

	create(old_swap_chain);
}

void SwapChain::recreate()
{
	Graphics::logical_device->wait();
	this->images.clear();
	VkSwapchainKHR old_swapchain = swap_chain;
	create(old_swapchain);
	Graphics::logical_device->destroySwapChain(old_swapchain);
}

bool SwapChain::acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence)
{
	VkResult result = Graphics::logical_device->acquireNextImage(swap_chain, UINT64_MAX, signal_semaphore, signal_fence, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return false;
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		SK_ERROR("Vulkan: Couldn't acquire next swap chain image");
		return false;
	}
	return true;
}

bool SwapChain::present(VkSemaphore wait_semaphore)
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swap_chain;
	present_info.pImageIndices = &image_index;
	present_info.pWaitSemaphores = &wait_semaphore;
	present_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
	VkResult result = Graphics::logical_device->getPresentQueue().present(present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return false;
	else if (result != VK_SUCCESS)
	{
		SK_ERROR("Vulkan: Couldn't present a swap chain image");
		return false;
	}
	return true;
}

void SwapChain::create(VkSwapchainKHR old_swap_chain)
{
	const auto& caps = surface.getCapabilities();
	extent = VkExtent2D
	(
		std::clamp(caps.currentExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
		std::clamp(caps.currentExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height)
	);
	if (extent.width == 0 || extent.height == 0)
		return;

	uint32_t image_count = caps.minImageCount + 1;
	if (caps.maxImageCount > 0)
		image_count = std::min(image_count, caps.maxImageCount);

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = surface;
	ci.minImageCount = image_count;
	ci.imageFormat = VkFormat(getFormat());
	ci.imageColorSpace = color_space;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1; //For stereoscopic 3D apps
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ci.preTransform = caps.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //For transparent windows
	ci.presentMode = present_mode;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = old_swap_chain; //Necessary for resizing and such

	// Enable transfer source on swap chain images if supported
	if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	else SK_WARN("Swap chain doesn't support trasfer source usage");

	// Enable transfer destination on swap chain images if supported
	if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	else SK_WARN("Swap chain doesn't support trasfer destination usage");

	uint32_t queue_family_indices[] = { Graphics::physical_device->getGraphicsQueue(), Graphics::physical_device->getPresentQueue() };
	if (Graphics::physical_device->getGraphicsQueue() != Graphics::physical_device->getPresentQueue())
	{
		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		ci.queueFamilyIndexCount = countof(queue_family_indices);
		ci.pQueueFamilyIndices = queue_family_indices;
	}
	else ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	swap_chain = Graphics::logical_device->createSwapChain(ci);

	std::vector<VkImage> images = Graphics::logical_device->getSwapChainImages(swap_chain);
	this->images.resize(image_count);

	for (size_t i = 0; i < images.size(); ++i)
		this->images[i] = makeShared<Image>(getWidth(), getHeight(), getFormat(), images[i]);
}

void SwapChain::destroy()
{
	Graphics::logical_device->destroySwapChain(swap_chain);
}

SwapChain::~SwapChain()
{
	destroy();
}