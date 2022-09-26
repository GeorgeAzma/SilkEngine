#include "queue_family.h"
#include "gfx/graphics.h"

QueueFamily::QueueFamily(uint32_t index, const std::vector<float>& priorities) 
	: index(index), priorities(priorities), queues(priorities.size(), nullptr)
{
	ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	ci.queueFamilyIndex = this->index;
	ci.queueCount = this->priorities.size();
	ci.pQueuePriorities = this->priorities.data();
}

shared<Queue> QueueFamily::getQueue(VkDevice logical_device, uint32_t index)
{
	if (!queues[index].get())
	{
		queues[index] = makeShared<Queue>(priorities[index]);
		vkGetDeviceQueue(logical_device, this->index, index, &queues[index]->queue);
	}
	return queues[index];
}
