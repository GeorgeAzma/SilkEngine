#include "vertex_buffer.h"
#include "gfx/graphics.h"
#include "gfx/buffers/command_buffer.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize size, VmaMemoryUsage memory_usage)
	: Buffer(size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		memory_usage)
{
	setData(data, size);
}

void VertexBuffer::bind(size_t binding)
{
	Graphics::getActiveCommandBuffer().bindVertexBuffers(binding, { buffer });
}
