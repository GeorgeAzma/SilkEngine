#include "swap_chain.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/devices/logical_device.h"

SwapChain::SwapChain(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	//Choose format
	std::multimap<int, VkSurfaceFormatKHR> surface_formats;
	for (const auto& available_format : Graphics::physical_device->getSurfaceFormats())
	{
		int score = (available_format.format == VK_FORMAT_B8G8R8A8_UNORM) * 1000 +
			(available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) * 400;
		if (score >= 0)
			surface_formats.insert(std::make_pair(score, available_format));
	}
	SK_ASSERT(surface_formats.rbegin()->first >= 0, "Vulkan: Couldn't find supported formats to choose from");
	this->surface_format = surface_formats.rbegin()->second;

	//Choose present mode
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& available_present_mode : Graphics::physical_device->getPresentModes())
	{
		if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			present_mode = available_present_mode;
			break;
		}
	}
	this->present_mode = present_mode;

	depth_format = Graphics::physical_device->getDepthFormat();

	sample_count = Graphics::physical_device->getMaxSampleCount();

	render_pass = makeShared<RenderPass>();
	render_pass->addSubpass()
		.addAttachment({ surface_format.format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {}, sample_count })
		.addAttachment({ depth_format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, {}, sample_count })
		.addAttachment({ surface_format.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR })
		.build();

	create(old_swap_chain);
}

void SwapChain::recreate()
{
	Graphics::logical_device->waitIdle();
	VkSwapchainKHR old_swapchain = swap_chain;
	create(old_swapchain);
	Graphics::logical_device->destroySwapChain(old_swapchain);
}

void SwapChain::createFramebuffers()
{
	Image2DProps props{};
	props.width = extent.width;
	props.height = extent.height;
	props.format = ImageFormatEnum::fromVulkanType(Graphics::physical_device->getDepthFormat());
	props.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	props.create_sampler = false;
	props.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	props.mipmap = false;
	props.samples = sample_count;
	depth = makeShared<Image2D>(props);

	props.format = ImageFormatEnum::fromVulkanType(surface_format.format);
	props.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	props.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	msaa_image = makeShared<Image2D>(props);

	for (size_t i = 0; i < images.size(); ++i)
		framebuffers[i] = makeShared<Framebuffer>(*render_pass, std::vector<shared<Image2D>>{ msaa_image, depth, images[i] }, extent.width, extent.height);
}

void SwapChain::acquireNextImage(VkSemaphore signal_semaphore, VkFence signal_fence)
{
	auto result = Graphics::logical_device->acquireNextImage(swap_chain, UINT64_MAX, signal_semaphore, signal_fence, &image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		recreate();
}

VkResult SwapChain::present(VkSemaphore wait_semaphore)
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swap_chain;
	present_info.pImageIndices = &image_index;
	present_info.pWaitSemaphores = &wait_semaphore;
	present_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;

	return Graphics::logical_device->getPresentQueue().present(present_info);
}

void SwapChain::create(const std::optional<VkSwapchainKHR>& old_swap_chain)
{
	int width, height;
	glfwGetFramebufferSize(Window::getGLFWWindow(), &width, &height);
	this->extent = VkExtent2D
	(
		std::clamp((uint32_t)width,
			Graphics::physical_device->getSurfaceCapabilities().minImageExtent.width,
			Graphics::physical_device->getSurfaceCapabilities().maxImageExtent.width),
		std::clamp((uint32_t)height,
			Graphics::physical_device->getSurfaceCapabilities().minImageExtent.height,
			Graphics::physical_device->getSurfaceCapabilities().maxImageExtent.height)
	);

	uint32_t image_count = Graphics::physical_device->getSurfaceCapabilities().minImageCount + 1;
	if (Graphics::physical_device->getSurfaceCapabilities().maxImageCount > 0)
		image_count = std::min(image_count, Graphics::physical_device->getSurfaceCapabilities().maxImageCount);

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = *Graphics::surface;
	ci.minImageCount = image_count;
	ci.imageFormat = surface_format.format;
	ci.imageColorSpace = surface_format.colorSpace;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1; //For stereoscopic 3D apps
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	ci.preTransform = Graphics::physical_device->getSurfaceCapabilities().currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //For transparent windows
	ci.presentMode = present_mode;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = old_swap_chain ? *old_swap_chain : VK_NULL_HANDLE; //Necessary for resizing and such

	// Enable transfer source on swap chain images if supported
	if (Graphics::physical_device->getSurfaceCapabilities().supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	else SK_WARN("Swap chain doesn't support trasfer source usage");

	// Enable transfer destination on swap chain images if supported
	if (Graphics::physical_device->getSurfaceCapabilities().supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		ci.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	else SK_WARN("Swap chain doesn't support trasfer destination usage");

	const auto& indices = Graphics::physical_device->getQueueFamilyIndices();
	if (indices.graphics != indices.present)
	{
		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		auto queue_family_indices = indices.getIndices();
		ci.queueFamilyIndexCount = queue_family_indices.size();
		ci.pQueueFamilyIndices = queue_family_indices.data();
	}
	else
		ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	swap_chain = Graphics::logical_device->createSwapChain(ci);

	std::vector<VkImage> images = Graphics::logical_device->getSwapChainImages(swap_chain);
	this->images.resize(image_count);

	for (size_t i = 0; i < images.size(); ++i)
	{
		Image2DProps props{};
		props.width = extent.width;
		props.height = extent.height;
		props.create_sampler = false;
		props.mipmap = false;
		props.format = ImageFormatEnum::fromVulkanType(surface_format.format);
		props.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		props.initial_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		this->images[i] = makeShared<Image2D>(images[i], props);
	}
	framebuffers.resize(images.size());

	createFramebuffers();
}

void SwapChain::destroy()
{
	Graphics::logical_device->destroySwapChain(swap_chain);
}

SwapChain::~SwapChain()
{
	destroy();
}