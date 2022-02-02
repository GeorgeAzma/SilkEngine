#pragma once

class LogicalDevice : NonCopyable
{
public:
	LogicalDevice();
	~LogicalDevice();

	operator const VkDevice& () const { return logical_device; }
	const VkQueue& getGraphicsQueue() const { return graphics_queue; }
	const VkQueue& getTransferQueue() const { return transfer_queue; }
	const VkQueue& getPresentQueue() const { return present_queue; }
	const VkQueue& getComputeQueue() const { return compute_queue; }

	static std::vector<const char*> getRequiredLogicalDeviceExtensions();

private:
	VkQueue graphics_queue;
	VkQueue transfer_queue;
	VkQueue present_queue;
	VkQueue compute_queue;
	VkDevice logical_device;
};