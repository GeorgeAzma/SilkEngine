#pragma once

class PhysicalDevice
{
public:
	PhysicalDevice(VkInstance* instance);
	~PhysicalDevice();

private:
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
	void chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	int ratePhysicalDevice(VkPhysicalDevice physical_device) const;

private:
	VkInstance* instance = nullptr;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
};