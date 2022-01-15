#include "vertex_buffer.h"
#include "gfx/graphics.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize size)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY)
{
	StagingBuffer staging_buffer(data, size);
	staging_buffer.copy(buffer);
}

void VertexBuffer::bind()
{
	if (Graphics::active.vertex_buffer == buffer)
		return;

	constexpr VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(Graphics::active.command_buffer, 0, 1, &buffer, &offset);
}
