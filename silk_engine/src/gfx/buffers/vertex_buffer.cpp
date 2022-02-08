#include "vertex_buffer.h"
#include "gfx/graphics.h"

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
	constexpr VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(Graphics::active.command_buffer, binding, 1, &buffer, &offset);
}
