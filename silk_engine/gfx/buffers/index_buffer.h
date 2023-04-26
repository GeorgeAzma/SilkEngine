#pragma once

#include "buffer.h"

enum class IndexType
{
	UINT16 = VK_INDEX_TYPE_UINT16,
	UINT32 = VK_INDEX_TYPE_UINT32
};

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const void* data, VkDeviceSize count, IndexType index_type = IndexType::UINT32);

	void bind(VkDeviceSize offset = 0) const;

	IndexType getIndexType() const { return index_type; }
	VkDeviceSize getCount() const { return ci.size / indexTypeSize(index_type); }

private:
	static size_t indexTypeSize(IndexType index_type)
	{
		return size_t(index_type) * 2 + 2;
	}

private:
	IndexType index_type;
};