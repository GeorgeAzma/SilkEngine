#include "surface.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/instance.h"
#include "window.h"
#include <GLFW/glfw3.h>

Surface::Surface(const Window& window)
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(*Graphics::instance, window, nullptr, &surface));

	updateCapabilities();
	formats = Graphics::physical_device->getSurfaceFormats(surface);
	present_modes = Graphics::physical_device->getSurfacePresentModes(surface);
}

Surface::~Surface()
{
	Graphics::instance->destroySurface(surface);
}

void Surface::updateCapabilities()
{
	capabilities = Graphics::physical_device->getSurfaceCapabilities(surface);
}
