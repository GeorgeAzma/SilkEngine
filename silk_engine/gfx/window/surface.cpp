#include "surface.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "gfx/instance.h"
#include "window.h"

Surface::Surface(const Window& window)
{
	RenderContext::vulkanAssert(glfwCreateWindowSurface(RenderContext::getInstance(), window, nullptr, &surface));

	updateCapabilities();
	formats = RenderContext::getPhysicalDevice().getSurfaceFormats(surface);
	present_modes = RenderContext::getPhysicalDevice().getSurfacePresentModes(surface);

	if (!isSupported())
	{
		SK_ERROR("Vulkan: Physical device does not support window surface");
		return;
	}

	int max_score = -1;
	VkSurfaceFormatKHR best_surface_format{};
	for (const auto& available_format : formats)
	{
		int score = 0;
		score += (available_format.format == VK_FORMAT_B8G8R8A8_UNORM) * 1000;
		score += (available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) * 400;
		if (score > max_score)
		{
			max_score = max(max_score, score);
			best_surface_format = available_format;
		}
	}
	format = best_surface_format;

	// Find queue family that supports both present and graphics queue, if there is none, then find queue family with only present queue
	const auto& queue_family_props = RenderContext::getPhysicalDevice().getQueueFamilyProperties();
	for (size_t i = 0; i < queue_family_props.size(); ++i)
	{
		if (RenderContext::getPhysicalDevice().getSurfaceSupport(i, surface))
		{
			present_queue = i;
			if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				break;
		}

	}
}

Surface::~Surface()
{
	RenderContext::getInstance().destroySurface(surface);
}

void Surface::updateCapabilities()
{
	capabilities = RenderContext::getPhysicalDevice().getSurfaceCapabilities(surface);
}
