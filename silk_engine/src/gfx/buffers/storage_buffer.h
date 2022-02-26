#pragma once

#include "buffer.h"

class StorageBuffer : public Buffer
{
public:
	StorageBuffer(vk::DeviceSize size, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU, vk::BufferUsageFlags usage_flags = vk::BufferUsageFlags(0));
};