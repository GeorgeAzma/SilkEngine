#include "surface.h"
#include "core/event.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/instance.h"
#include "gfx/devices/physical_device.h"

Surface::Surface()
{
	VkSurfaceKHR vk_surface = surface;
	Graphics::vulkanAssert(glfwCreateWindowSurface(vk::Instance(*Graphics::instance), Window::getGLFWWindow(), nullptr, &vk_surface));
}

Surface::~Surface()
{
	Graphics::instance->destroySurface(surface);
}