#include "queue.h"

Queue::Queue(float priority) : priority(priority)
{
}

VkResult Queue::present(const VkPresentInfoKHR& present_info) const
{
	return vkQueuePresentKHR(queue, &present_info);
}