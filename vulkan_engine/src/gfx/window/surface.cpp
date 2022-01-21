#include "surface.h"
#include "gfx/graphics.h"
#include "gfx/window/window.h"

Surface::Surface()
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(*Graphics::instance, Window::getGLFWWindow(), nullptr, &surface));
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(*Graphics::instance, surface, nullptr);
}
