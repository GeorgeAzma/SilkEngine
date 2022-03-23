#include "vertex_buffer.h"
#include "gfx/graphics.h"
#include "gfx/buffers/command_buffer.h"

VertexBuffer::VertexBuffer(const void* data, vk::DeviceSize size, VmaMemoryUsage memory_usage)
	: Buffer(size,
		vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eVertexBuffer,
		memory_usage)
{
	setData(data, size);
}

void VertexBuffer::bind(size_t binding)
{
	constexpr vk::DeviceSize offset = 0;
	Graphics::getActiveCommandBuffer().bindVertexBuffers(binding, { buffer }, { offset });
}
