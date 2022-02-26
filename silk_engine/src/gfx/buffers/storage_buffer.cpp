#include "storage_buffer.h"

StorageBuffer::StorageBuffer(vk::DeviceSize size, VmaMemoryUsage memory_usage, vk::BufferUsageFlags usage_flags)
	: Buffer(size, 
		vk::BufferUsageFlagBits::eStorageBuffer | usage_flags,
		memory_usage)
{
}