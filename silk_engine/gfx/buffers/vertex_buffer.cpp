#include "vertex_buffer.h"
#include "gfx/render_context.h"
#include "gfx/buffers/command_buffer.h"

VertexBuffer::VertexBuffer(const void* data, VkDeviceSize vertex_size, uint32_t vertex_count, bool instanced)
	: Buffer(vertex_size * vertex_count, instanced ? VERTEX : (VERTEX | TRANSFER_DST), instanced ? Allocation::Props{ Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED, Allocation::Device::CPU } : Allocation::Props{}),
	vertex_size(vertex_size), vertex_count(vertex_count)
{
	setData(data);
}

void VertexBuffer::bind(uint32_t binding, VkDeviceSize offset)
{
	RenderContext::record([&](CommandBuffer& cb) { cb.bindVertexBuffers(binding, { buffer }, { offset }); });
}
