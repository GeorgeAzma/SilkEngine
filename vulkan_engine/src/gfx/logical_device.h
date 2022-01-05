#pragma once

class LogicalDevice
{
public:
	LogicalDevice();
	~LogicalDevice();

	operator const VkDevice& () const { return logical_device; }
	const VkQueue& getGraphicsQueue() const { return graphics_queue; }
	const VkQueue& getPresentQueue() const { return present_queue; }

	static std::vector<const char*> getRequiredLogicalDeviceExtensions();

private:
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkDevice logical_device;
};