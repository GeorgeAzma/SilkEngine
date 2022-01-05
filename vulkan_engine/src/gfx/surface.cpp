#include "surface.h"
#include "graphics.h"

Surface::Surface(GLFWwindow* window)
{
	VE_CORE_ASSERT(glfwCreateWindowSurface(Graphics::getInstance()->getInstance(), window, nullptr, &surface) == VK_SUCCESS,
		"Couldn't create window surface");

}

Surface::~Surface()
{
	vkDestroySurfaceKHR(Graphics::getInstance()->getInstance(), surface, nullptr);
}
