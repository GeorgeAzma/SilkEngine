#pragma once

class PhysicalDevice
{
public:
	PhysicalDevice(VkInstance* instance);
	~PhysicalDevice();

private:
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
	bool isDeviceSuitable(VkPhysicalDevice physical_device) const;
	void chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);

private:
	VkInstance* instance = nullptr;
	VkPhysicalDevice physica_device = VK_NULL_HANDLE;
};