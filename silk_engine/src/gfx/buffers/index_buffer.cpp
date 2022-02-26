#include "index_buffer.h"
#include "staging_buffer.h"
#include "gfx/graphics.h"

IndexBuffer::IndexBuffer(const void* data, vk::DeviceSize count, IndexType index_type, VmaMemoryUsage memory_usage)
	: Buffer(count * EnumInfo::size(index_type),
		vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eIndexBuffer,
		memory_usage),
	index_type(EnumInfo::indexType(index_type))
{
	setData(data, count * EnumInfo::size(index_type));
}

void IndexBuffer::bind()
{
	Graphics::active.command_buffer.bindIndexBuffer(buffer, 0, index_type);
}
