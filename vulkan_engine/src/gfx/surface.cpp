#include "surface.h"
#include "graphics.h"
#include "core/window.h"

Surface::Surface()
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(*Graphics::instance, Window::getGLFWWindow(), nullptr, &surface));
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(*Graphics::instance, surface, nullptr);
}
