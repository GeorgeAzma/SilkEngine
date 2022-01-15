#include "index_buffer.h"
#include "staging_buffer.h"
#include "gfx/graphics.h"

IndexBuffer::IndexBuffer(const void* data, VkDeviceSize size, IndexType index_type)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VMA_MEMORY_USAGE_GPU_ONLY), 
	index_type{EnumInfo::indexType(index_type)}
{
	StagingBuffer staging_buffer(data, size);
	staging_buffer.copy(buffer);
}

void IndexBuffer::bind()
{
	if (Graphics::active.index_buffer == buffer)
		return;

	vkCmdBindIndexBuffer(Graphics::active.command_buffer, buffer, 0, index_type);
}
