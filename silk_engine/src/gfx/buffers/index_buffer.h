#pragma once

#include "buffer.h"

enum class IndexType
{
	UINT16,
	UINT32
};

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const void* data, VkDeviceSize count, IndexType index_type = IndexType::UINT32, VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_GPU_ONLY);

	void bind(VkDeviceSize offset = 0);

	IndexType getIndexType() const { return index_type; }
	VkDeviceSize getCount() const { return size / indexTypeSize(index_type); }

private:
	static VkIndexType indexType(IndexType index_type);
	static size_t indexTypeSize(IndexType index_type);

private:
	IndexType index_type;
};