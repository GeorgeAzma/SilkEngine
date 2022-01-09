#include "surface.h"
#include "graphics.h"

Surface::Surface()
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(Graphics::instance->getInstance(), Graphics::window, nullptr, &surface));
}

void Surface::getSupportDetails(VkPhysicalDevice physical_device)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

	if (format_count)
	{
		formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());
	}

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

	if (present_mode_count)
	{
		present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
	}
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(Graphics::instance->getInstance(), surface, nullptr);
}
