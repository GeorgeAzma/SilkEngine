#include "index_buffer.h"
#include "staging_buffer.h"

IndexBuffer::IndexBuffer(const void* data, VkDeviceSize size, IndexType index_type)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), 
	index_type{EnumInfo::indexType(index_type)}
{
	StagingBuffer staging_buffer(data, size);
	staging_buffer.copy(buffer);
}

void IndexBuffer::bind(VkCommandBuffer command_buffer)
{
	vkCmdBindIndexBuffer(command_buffer, buffer, 0, index_type);
}
