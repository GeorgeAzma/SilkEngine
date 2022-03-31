#include "storage_buffer.h"

StorageBuffer::StorageBuffer(VkDeviceSize size, VmaMemoryUsage memory_usage, VkBufferUsageFlags usage_flags)
	: Buffer(size, 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usage_flags,
		memory_usage)
{
}