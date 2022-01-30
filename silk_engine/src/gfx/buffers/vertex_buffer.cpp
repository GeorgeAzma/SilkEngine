#include "vertex_buffer.h"
#include "gfx/graphics.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize size, VmaMemoryUsage memory_usage)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		memory_usage)
{
	if (memory_usage == VMA_MEMORY_USAGE_GPU_ONLY)
	{
		StagingBuffer staging_buffer(data, size);
		staging_buffer.copy(buffer);
	}
	else
	{
		setData(data, size);
	}
}

void VertexBuffer::bind(size_t binding)
{
	constexpr VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(Graphics::active.command_buffer, binding, 1, &buffer, &offset);
}
