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
