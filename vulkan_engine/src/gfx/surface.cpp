#include "surface.h"

Surface::Surface(const VkInstance* instance, GLFWwindow* window)
	: instance{instance}
{
	VE_CORE_ASSERT(glfwCreateWindowSurface(*instance, window, nullptr, &surface) == VK_SUCCESS,
		"Couldn't create window surface");

}

Surface::~Surface()
{
	vkDestroySurfaceKHR(*instance, surface, nullptr);
}
