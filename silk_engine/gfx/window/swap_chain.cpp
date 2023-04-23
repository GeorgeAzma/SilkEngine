#include "swap_chain.h"
#include "gfx/devices/physical_device.h"
#include "gfx/render_context.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/devices/logical_device.h"
#include "gfx/queue.h"

SwapChain::SwapChain(const Surface& surface, bool vsync)
	: surface(surface)
{
	if (!surface.isSupported())
		return;
	recreate(vsync);
}

bool SwapChain::acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence) const
{
	VkResult result = RenderContext::getLogicalDevice().acquireNextImage(swap_chain, UINT64_MAX, signal_semaphore, signal_fence, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		return false;
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		SK_ERROR("Vulkan: Couldn't acquire next swap chain image");
		return false;
	}
	return true;
}

bool SwapChain::present(VkSemaphore wait_semaphore) const
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swap_chain;
	present_info.pImageIndices = &image_index;
	present_info.pWaitSemaphores = &wait_semaphore;
	present_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
	VkResult result = RenderContext::getLogicalDevice().getPresentQueue(surface).present(present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		return false;
	else if (result != VK_SUCCESS)
	{
		SK_ERROR("Vulkan: Couldn't present a swap chain image");
		return false;
	}
	return true;
}

void SwapChain::recreate(bool vsync)
{
	const auto& caps = surface.getCapabilities();
	extent = VkExtent2D
	(
		std::clamp(caps.currentExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width),
		std::clamp(caps.currentExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height)
	);
	if (extent.width == 0 || extent.height == 0)
		return;

	RenderContext::getLogicalDevice().wait();
	this->images.clear();

	VkSwapchainKHR old_swap_chain = swap_chain;
	swap_chain = nullptr;

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // Gauranteed to be supported
	if (!vsync)
	{
		for (const auto& available_present_mode : surface.getPresentModes())
		{
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				present_mode = available_present_mode;
				break;
			}
			if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}
	this->present_mode = present_mode;

	uint32_t image_count = caps.minImageCount + 1;
	if (caps.maxImageCount > 0)
		image_count = std::min(image_count, caps.maxImageCount);

	VkSurfaceTransformFlagsKHR transform = caps.currentTransform;
	if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		transform = caps.currentTransform;

	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Choose non opaque composite alpha for transparent windows
	VkCompositeAlphaFlagBitsKHR composite_alpha_flags[] = 
	{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (auto& composite_alpha_flag : composite_alpha_flags) 
	{
		if (caps.supportedCompositeAlpha & composite_alpha_flag)
		{
			composite_alpha = composite_alpha_flag;
			break;
		}
	}

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = surface;
	ci.minImageCount = image_count;
	ci.imageFormat = surface.getFormat().format;
	ci.imageColorSpace = surface.getFormat().colorSpace;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1; //For stereoscopic 3D apps
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ci.preTransform = (VkSurfaceTransformFlagBitsKHR)transform;
	ci.compositeAlpha = composite_alpha;
	ci.presentMode = present_mode;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = old_swap_chain;

	// Enable transfer source on swap chain images if supported
	if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	else SK_WARN("Swap chain doesn't support trasfer source usage");

	// Enable transfer destination on swap chain images if supported
	if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	else SK_WARN("Swap chain doesn't support trasfer destination usage");

	uint32_t queue_family_indices[] = { RenderContext::getPhysicalDevice().getGraphicsQueue(), surface.getPresentQueue() };
	if (queue_family_indices[0] != queue_family_indices[1])
	{
		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		ci.queueFamilyIndexCount = countof(queue_family_indices);
		ci.pQueueFamilyIndices = queue_family_indices;
	}
	else ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	swap_chain = RenderContext::getLogicalDevice().createSwapChain(ci);

	std::vector<VkImage> images = RenderContext::getLogicalDevice().getSwapChainImages(swap_chain);
	this->images.resize(image_count);

	for (size_t i = 0; i < images.size(); ++i)
		this->images[i] = makeShared<Image>(getWidth(), getHeight(), Image::Format(surface.getFormat().format), images[i]);

	if (old_swap_chain)
	{
		RenderContext::getLogicalDevice().destroySwapChain(old_swap_chain);
		old_swap_chain = nullptr;
	}
}

SwapChain::~SwapChain()
{
	RenderContext::getLogicalDevice().destroySwapChain(swap_chain);
}