#pragma once

#include "buffer.h"
#include "staging_buffer.h"

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(const void* data, VkDeviceSize size, VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY);

	void bind(size_t binding = 0);
};