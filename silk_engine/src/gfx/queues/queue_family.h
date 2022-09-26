#pragma once

#include "queue.h"

class QueueFamily
{
public:
	QueueFamily(uint32_t index, const std::vector<float>& priorities);

	shared<Queue> getQueue(VkDevice logical_device, uint32_t index);

	operator const VkDeviceQueueCreateInfo& () const { return ci; }

private:
	uint32_t index;
	std::vector<shared<Queue>> queues;
	VkDeviceQueueCreateInfo ci{};
	std::vector<float> priorities;
};