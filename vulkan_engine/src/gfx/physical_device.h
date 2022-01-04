#pragma once
#include "queue_family.h"

class PhysicalDevice
{
public:
	PhysicalDevice(VkInstance* instance);
	~PhysicalDevice();

private:
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
	void chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	int ratePhysicalDevice(VkPhysicalDevice physical_device);

private:
	VkInstance* instance = nullptr;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	QueueFamily* queue_family = nullptr;
};