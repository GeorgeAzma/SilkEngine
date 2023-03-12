#include "queue.h"
#include "gfx/fence.h"

Queue::Queue(VkQueue queue, float priority) 
	: queue(queue), priority(priority)
{
}

VkResult Queue::present(const VkPresentInfoKHR& present_info) const
{
	return vkQueuePresentKHR(queue, &present_info);
}

void Queue::submit(const VkSubmitInfo& submit_info, VkFence fence) const
{
	vkQueueSubmit(queue, 1, &submit_info, fence);
}

void Queue::submitImmidiatly(const VkSubmitInfo& submit_info) const
{
	Fence fence;
	vkQueueSubmit(queue, 1, &submit_info, fence);
	fence.wait();
}

void Queue::wait() const
{
	vkQueueWaitIdle(queue);
}
