#pragma once

#include "buffer.h"
#include "gfx/enums.h"

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const void* data, vk::DeviceSize count, IndexType index_type = IndexType::UINT32, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY);

	void bind();

	IndexType getIndexType() const { return index_type; }
	vk::DeviceSize getCount() const { return size / EnumInfo::size(index_type); }

private:
	IndexType index_type;
};