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
	surface = new Surface(window);
	physical_device = new PhysicalDevice();
	logical_device = new LogicalDevice();
	swap_chain = new SwapChain(window);
}

void Graphics::cleanup()
{
	delete swap_chain;
	delete logical_device;
	delete physical_device;
	delete surface;
	delete instance;
}
