#include "index_buffer.h"
#include "gfx/render_context.h"
#include "gfx/buffers/command_buffer.h"

IndexBuffer::IndexBuffer(const void* data, VkDeviceSize count, IndexType index_type)
	: Buffer(count * indexTypeSize(index_type), INDEX | TRANSFER_DST), index_type(index_type)
{
	setData(data, count * indexTypeSize(index_type));
}

void IndexBuffer::bind(VkDeviceSize offset) const
{
	RenderContext::submit([&](CommandBuffer& cb) { cb.bindIndexBuffer(buffer, offset, indexType(index_type)); });
}

VkIndexType IndexBuffer::indexType(IndexType index_type)
{
	switch (index_type)
	{
		case IndexType::UINT16: return VK_INDEX_TYPE_UINT16;
		case IndexType::UINT32: return VK_INDEX_TYPE_UINT32;
	}

	return VkIndexType(0);
}

size_t IndexBuffer::indexTypeSize(IndexType index_type)
{
	switch (index_type)
	{
		case IndexType::UINT16: return 2;
		case IndexType::UINT32: return 4;
	}

	return 0;
}