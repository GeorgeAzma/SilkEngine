#pragma once

class Queue
{
	friend class QueueFamily;

public:
	Queue(float priority = 1.0f);

	VkResult present(const VkPresentInfoKHR& present_info) const;

	operator const VkQueue& () const { return queue; }

private:
	VkQueue queue = VK_NULL_HANDLE;
	float priority;
};