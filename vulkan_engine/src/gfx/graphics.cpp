#include "graphics.h"

Instance* Graphics::instance = nullptr;
Surface* Graphics::surface = nullptr;
PhysicalDevice* Graphics::physical_device = nullptr;
LogicalDevice* Graphics::logical_device = nullptr;
SwapChain* Graphics::swap_chain = nullptr;

void Graphics::init(GLFWwindow* window)
{
	VE_CORE_ASSERT(!instance, "Vulkan: Reinitializing vulkan instance is not allowed");
	
	instance = new Instance();
	surface = new Surface(&instance->getInstance(), window);
	physical_device = new PhysicalDevice(&instance->getInstance(), &surface->getSurface());
	logical_device = new LogicalDevice(&physical_device->getPhysicalDevice(), &surface->getSurface());
	swap_chain = new SwapChain(&physical_device->getPhysicalDevice(), &surface->getSurface(), window, &logical_device->getLogicalDevice());
}

void Graphics::cleanup()
{
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;
}
