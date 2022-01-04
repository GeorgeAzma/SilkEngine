#pragma once
#include "queue_family.h"

class PhysicalDevice
{
public:
	PhysicalDevice(VkInstance* instance, VkSurfaceKHR* surface);
	~PhysicalDevice();

	VkPhysicalDevice& getPhysicalDevice() { return physical_device; }

private:
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
	void chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	int ratePhysicalDevice(VkPhysicalDevice physical_device);

private:
	VkInstance* instance = nullptr;
	VkSurfaceKHR* surface = nullptr;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	QueueFamily queue_family;
	friend class QueueFamily;
};