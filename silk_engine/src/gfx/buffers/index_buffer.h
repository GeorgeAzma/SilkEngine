#pragma once

#include "buffer.h"
#include "gfx/enums.h"

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const void* data, vk::DeviceSize count, IndexType index_type = IndexType::UINT32, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY);

	void bind();

private:
	vk::IndexType index_type;
};