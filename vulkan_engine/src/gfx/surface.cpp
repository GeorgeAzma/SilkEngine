#include "surface.h"
#include "graphics.h"

Surface::Surface()
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(*Graphics::instance, Graphics::window, nullptr, &surface));
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(*Graphics::instance, surface, nullptr);
}
