#pragma once

class LogicalDevice
{
public:
	LogicalDevice(const VkPhysicalDevice* physical_device, const VkSurfaceKHR* surface);
	~LogicalDevice();

	const VkDevice& getLogicalDevice() const { return logical_device; }

	static std::vector<const char*> getRequiredLogicalDeviceExtensions();

private:
	const VkPhysicalDevice* physical_device = nullptr;
	const VkSurfaceKHR* surface = nullptr;
	VkDevice logical_device;
	VkQueue graphics_queue;
	VkQueue present_queue;
};