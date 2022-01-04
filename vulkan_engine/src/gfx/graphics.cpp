#include "graphics.h"

Instance* Graphics::instance = nullptr;
Surface* Graphics::surface = nullptr;

void Graphics::init(GLFWwindow* window)
{
	VE_CORE_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");
	instance = new Instance();
	surface = new Surface(&instance->getInstance(), window);
}

void Graphics::cleanup()
{
	delete surface;
	delete instance;
}
