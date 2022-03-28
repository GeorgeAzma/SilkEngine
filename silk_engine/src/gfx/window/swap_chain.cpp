#include "swap_chain.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/window/surface.h"
#include "gfx/devices/logical_device.h"
#include <GLFW/glfw3.h>

SwapChain::SwapChain(const std::optional<vk::SwapchainKHR>& old_swap_chain)
{
	//Choose format
	std::multimap<int, vk::SurfaceFormatKHR> surface_formats;
	for (const auto& available_format : Graphics::physical_device->getSurfaceFormats())
	{
		int score = (available_format.format == vk::Format::eB8G8R8A8Unorm) * 1000 +
			(available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) * 400;
		if (score >= 0)
			surface_formats.insert(std::make_pair(score, available_format));
	}
	SK_ASSERT(surface_formats.rbegin()->first >= 0, "Vulkan: Couldn't find supported formats to choose from");
	this->surface_format = surface_formats.rbegin()->second;

	//Choose present mode
	vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
	for (const auto& available_present_mode : Graphics::physical_device->getPresentModes())
	{
		if (available_present_mode == vk::PresentModeKHR::eMailbox)
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
		.addAttachment({ surface_format.format, vk::ImageLayout::eColorAttachmentOptimal, {}, sample_count })
		.addAttachment({ depth_format, vk::ImageLayout::eDepthStencilAttachmentOptimal, {}, sample_count })
		.addAttachment({ surface_format.format, vk::ImageLayout::ePresentSrcKHR })
		.build();

	create(old_swap_chain);
}

void SwapChain::recreate()
{
	Graphics::logical_device->waitIdle();
	vk::SwapchainKHR old_swapchain = swap_chain;
	create(old_swapchain);
	Graphics::logical_device->destroySwapChain(old_swapchain);
}

void SwapChain::createFramebuffers()
{
	Image2DProps props{};
	props.width = extent.width;
	props.height = extent.height;
	props.format = Graphics::physical_device->getDepthFormat();
	props.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	props.create_sampler = false;
	props.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	props.mipmap = false;
	props.samples = sample_count;
	depth = makeShared<Image2D>(props);

	props.format = surface_format.format;
	props.usage = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
	props.layout = vk::ImageLayout::eUndefined;
	msaa_image = makeShared<Image2D>(props);

	for (size_t i = 0; i < images.size(); ++i)
		framebuffers[i] = makeShared<Framebuffer>(*render_pass, std::vector<shared<Image2D>>{ msaa_image, depth, images[i] }, extent.width, extent.height);
}

void SwapChain::acquireNextImage(vk::Semaphore signal_semaphore, vk::Fence signal_fence)
{
	auto result = Graphics::logical_device->acquireNextImage(swap_chain, UINT64_MAX, signal_semaphore, signal_fence, &image_index);
	if (result == vk::Result::eErrorOutOfDateKHR)
		recreate();
}

vk::Result SwapChain::present(vk::Semaphore wait_semaphore)
{
	vk::PresentInfoKHR present_info{};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swap_chain;
	present_info.pImageIndices = &image_index;
	present_info.pWaitSemaphores = &wait_semaphore;
	present_info.waitSemaphoreCount = wait_semaphore ? 1 : 0;

	return Graphics::logical_device->getPresentQueue().presentKHR(present_info);
}

void SwapChain::create(const std::optional<vk::SwapchainKHR>& old_swap_chain)
{
	int width, height;
	glfwGetFramebufferSize(Window::getGLFWWindow(), &width, &height);
	this->extent = vk::Extent2D
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

	vk::SwapchainCreateInfoKHR ci{};
	ci.surface = *Graphics::surface;
	ci.minImageCount = image_count;
	ci.imageFormat = surface_format.format;
	ci.imageColorSpace = surface_format.colorSpace;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1; //For stereoscopic 3D apps
	ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	ci.preTransform = Graphics::physical_device->getSurfaceCapabilities().currentTransform;
	ci.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; //For transparent windows
	ci.presentMode = present_mode;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = old_swap_chain ? *old_swap_chain : VK_NULL_HANDLE; //Necessary for resizing and such

	// Enable transfer source on swap chain images if supported
	if (Graphics::physical_device->getSurfaceCapabilities().supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc)
		ci.imageUsage |= vk::ImageUsageFlagBits::eTransferSrc;
	else SK_WARN("Swap chain doesn't support trasfer source usage");

	// Enable transfer destination on swap chain images if supported
	if (Graphics::physical_device->getSurfaceCapabilities().supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
		ci.imageUsage |= vk::ImageUsageFlagBits::eTransferDst;
	else SK_WARN("Swap chain doesn't support trasfer destination usage");

	const auto& indices = Graphics::physical_device->getQueueFamilyIndices();
	if (indices.graphics != indices.present)
	{
		ci.imageSharingMode = vk::SharingMode::eConcurrent;
		auto queue_family_indices = indices.getIndices();
		ci.queueFamilyIndexCount = queue_family_indices.size();
		ci.pQueueFamilyIndices = queue_family_indices.data();
	}
	else
		ci.imageSharingMode = vk::SharingMode::eExclusive;

	swap_chain = Graphics::logical_device->createSwapChain(ci);

	std::vector<vk::Image> images = Graphics::logical_device->getSwapChainImages(swap_chain);
	this->images.resize(image_count);

	for (size_t i = 0; i < images.size(); ++i)
	{
		Image2DProps props{};
		props.width = extent.width;
		props.height = extent.height;
		props.create_sampler = false;
		props.mipmap = false;
		props.format = surface_format.format;
		props.layout = vk::ImageLayout::ePresentSrcKHR;
		props.initial_layout = vk::ImageLayout::ePresentSrcKHR;
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