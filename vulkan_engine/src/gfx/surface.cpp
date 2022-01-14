#include "surface.h"
#include "graphics.h"

Surface::Surface()
{
	Graphics::vulkanAssert(glfwCreateWindowSurface(Graphics::instance->getInstance(), Graphics::window, nullptr, &surface));
}

Surface::~Surface()
{
	vkDestroySurfaceKHR(Graphics::instance->getInstance(), surface, nullptr);
}
