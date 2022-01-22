#include "storage_buffer.h"

StorageBuffer::StorageBuffer(VkDeviceSize size, VmaMemoryUsage memory_usage)
	: Buffer(size, 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		memory_usage)
{
}