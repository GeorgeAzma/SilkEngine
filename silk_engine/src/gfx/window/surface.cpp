#include "surface.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/instance.h"
#include "window.h"
#include <GLFW/glfw3.h>

Surface::Surface(const Window& window)
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(*Graphics::instance, window.getGLFWWindow(), nullptr, &surface));

	update(window.getWidth(), window.getHeight());

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*Graphics::physical_device, surface, &format_count, nullptr);
	formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(*Graphics::physical_device, surface, &format_count, formats.data());

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(*Graphics::physical_device, surface, &present_mode_count, nullptr);
	present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(*Graphics::physical_device, surface, &present_mode_count, present_modes.data());
}

Surface::~Surface()
{
	Graphics::instance->destroySurface(surface);
}

void Surface::update(uint32_t width, uint32_t height)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*Graphics::physical_device, surface, &capabilities);
}