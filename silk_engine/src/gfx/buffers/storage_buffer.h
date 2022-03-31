#pragma once

#include "buffer.h"

class StorageBuffer : public Buffer
{
public:
	StorageBuffer(VkDeviceSize size, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU, VkBufferUsageFlags usage_flags = VkBufferUsageFlags(0));
};