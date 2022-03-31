#include "index_buffer.h"
#include "staging_buffer.h"
#include "gfx/graphics.h"
#include "gfx/buffers/command_buffer.h"

IndexBuffer::IndexBuffer(const void* data, VkDeviceSize count, IndexType index_type, VmaMemoryUsage memory_usage)
	: Buffer(count * indexTypeSize(index_type),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		memory_usage), index_type(index_type)
{
	setData(data, count * indexTypeSize(index_type));
}

void IndexBuffer::bind(VkDeviceSize offset)
{
	Graphics::getActiveCommandBuffer().bindIndexBuffer(buffer, offset, indexType(index_type));
}

VkIndexType IndexBuffer::indexType(IndexType index_type)
{
	switch (index_type)
	{
		case IndexType::UINT16: return VK_INDEX_TYPE_UINT16;
		case IndexType::UINT32: return VK_INDEX_TYPE_UINT32;
	}

	SK_ERROR("Unsupported index type specified: {0}.", index_type);
	return VkIndexType(0);
}

size_t IndexBuffer::indexTypeSize(IndexType index_type)
{
	switch (index_type)
	{
		case IndexType::UINT16: return 2;
		case IndexType::UINT32: return 4;
	}

	SK_ERROR("Unsoppurted index type specified: {0}.", index_type);
	return 0;
}