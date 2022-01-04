#include "logical_device.h"

LogicalDevice::LogicalDevice(VkPhysicalDevice* physical_device, QueueFamily queue_family)
	: physical_device{physical_device}
{
	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = *queue_family.getIndices().graphics;
	queue_create_info.queueCount = 1; 
	float queue_priority = 1.0f;
	queue_create_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures device_features{};

	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = &queue_create_info;
	create_info.queueCreateInfoCount = 1;
	create_info.pEnabledFeatures = &device_features;

	VE_CORE_ASSERT(vkCreateDevice(*physical_device, &create_info, nullptr, &logical_device) == VK_SUCCESS, 
		"Vulkan: Couldn't create logical device");

	vkGetDeviceQueue(logical_device, *queue_family.getIndices().graphics, 0, &graphics_queue);
}

LogicalDevice::~LogicalDevice()
{
	vkDestroyDevice(logical_device, nullptr);
}
