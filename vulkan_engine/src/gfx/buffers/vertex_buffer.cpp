#include "vertex_buffer.h"
#include "gfx/graphics.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize size, VmaMemoryUsage usage)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		usage)
{
	StagingBuffer staging_buffer(data, size);
	staging_buffer.copy(buffer);
}

void VertexBuffer::bind(size_t binding)
{
	constexpr VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(Graphics::active.command_buffer, binding, 1, &buffer, &offset);
}
