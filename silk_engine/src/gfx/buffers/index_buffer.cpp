#include "index_buffer.h"
#include "staging_buffer.h"
#include "gfx/graphics.h"

IndexBuffer::IndexBuffer(const void* data, VkDeviceSize count, IndexType index_type, VmaMemoryUsage memory_usage)
	: Buffer(count * EnumInfo::size(index_type),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		memory_usage),
	index_type(EnumInfo::indexType(index_type))
{
	if (memory_usage == VMA_MEMORY_USAGE_GPU_ONLY)
	{
		StagingBuffer staging_buffer(data, count * EnumInfo::size(index_type));
		staging_buffer.copy(buffer);
	}
	else
	{
		setData(data, count * EnumInfo::size(index_type));
	}
}

void IndexBuffer::bind()
{
	vkCmdBindIndexBuffer(Graphics::active.command_buffer, buffer, 0, index_type);
}
