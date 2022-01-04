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
	std::vector<VkExtensionProperties> getAvailablePhysicalDeviceExtensions(VkPhysicalDevice physical_device) const;
	bool checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device) const;

private:
	VkInstance* instance = nullptr;
	VkSurfaceKHR* surface = nullptr;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	QueueFamily queue_family;
	friend class QueueFamily;
};