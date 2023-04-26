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
	RenderContext::record([&](CommandBuffer& cb) { cb.bindIndexBuffer(buffer, offset, VkIndexType(index_type)); });
}