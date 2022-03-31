#include "surface.h"
#include "core/event.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"
#include "gfx/instance.h"
#include "gfx/devices/physical_device.h"

Surface::Surface()
{
	VkSurfaceKHR& vk_surface = (VkSurfaceKHR&)surface;
	Graphics::vulkanAssert(vk::Result(glfwCreateWindowSurface(vk::Instance(*Graphics::instance), Window::getGLFWWindow(), nullptr, &vk_surface)));
}

Surface::~Surface()
{
	Graphics::instance->destroySurface(surface);
}