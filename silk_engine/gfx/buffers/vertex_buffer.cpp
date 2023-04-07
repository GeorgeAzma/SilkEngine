#include "vertex_buffer.h"
#include "gfx/render_context.h"
#include "gfx/buffers/command_buffer.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize vertex_size, uint32_t vertex_count, bool instanced)
	: Buffer(vertex_size * vertex_count, VERTEX | TRANSFER_DST, instanced ? Allocation::Props{ Allocation::RANDOM_ACCESS | Allocation::MAPPED, Allocation::Device::CPU } : Allocation::Props{}),
	vertex_size(vertex_size), vertex_count(vertex_count)
{
	setData(data);
}

void VertexBuffer::bind(size_t binding)
{
	RenderContext::submit([&](CommandBuffer& cb) { cb.bindVertexBuffers(binding, { buffer }); });
}
