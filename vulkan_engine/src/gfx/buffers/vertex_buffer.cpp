#include "vertex_buffer.h"
#include "gfx/graphics.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize size)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
	StagingBuffer staging_buffer(data, size);
	staging_buffer.copy(buffer);
}

void VertexBuffer::bind()
{

}
