#pragma once
#include "instance.h"
#include "surface.h"
#include "physical_device.h"
#include "logical_device.h"
#include "swap_chain.h"

class Graphics
{
public:
	static void init(GLFWwindow* window);
	static void cleanup();

	static const Instance* getInstance() { return instance; }
	static const Surface* getSurface() { return surface; }
	static const PhysicalDevice* getPhysicalDevice() { return physical_device; }
	static const LogicalDevice* getLogicalDevice() { return logical_device; }

private:
	static Instance* instance;
	static Surface* surface;
	static PhysicalDevice* physical_device;
	static LogicalDevice* logical_device;
	static SwapChain* swap_chain;
};