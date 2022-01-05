#pragma once

class LogicalDevice
{
public:
	LogicalDevice();
	~LogicalDevice();

	VkDevice& getLogicalDevice() { return logical_device; }

	static std::vector<const char*> getRequiredLogicalDeviceExtensions();

private:
	VkDevice logical_device;
	VkQueue graphics_queue;
	VkQueue present_queue;
};