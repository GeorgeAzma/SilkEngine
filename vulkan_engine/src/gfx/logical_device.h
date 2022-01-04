#pragma once
#include "queue_family.h"

class LogicalDevice
{
public:
	LogicalDevice(VkPhysicalDevice* physical_device, QueueFamily queue_family);
	~LogicalDevice();

	static std::vector<const char*> getRequiredLogicalDeviceExtensions();

private:
	VkPhysicalDevice* physical_device = nullptr;
	VkDevice logical_device;
	VkQueue graphics_queue;
	VkQueue present_queue;
};