#include "vertex_array.h"
#include "gfx/render_context.h"
#include "gfx/buffers/command_buffer.h"

VertexArray::~VertexArray()
{
	for (auto& vertex_buffer : vertex_buffers)
		vertex_buffer = nullptr;
	index_buffer = nullptr;
}

VertexArray& VertexArray::addVertexBuffer(const shared<VertexBuffer>& vertex_buffer)
{
	vertex_buffers.emplace_back(vertex_buffer);
	vk_vertex_buffers.emplace_back(*vertex_buffer);
	return *this;
}

VertexArray& VertexArray::setIndexBuffer(const shared<IndexBuffer>& index_buffer)
{
	this->index_buffer = index_buffer;
	return *this;
}

void VertexArray::bind(const std::vector<VkDeviceSize>& offsets, VkDeviceSize index_offset) const
{
	RenderContext::record([&](CommandBuffer& cb) { cb.bindVertexBuffers(0, vk_vertex_buffers, offsets); });
	if (index_buffer)
		index_buffer->bind(index_offset);
}

void VertexArray::draw() const
{
	bind();
	if (index_buffer)
		RenderContext::record([&](CommandBuffer& cb) { cb.drawIndexed(index_buffer->getCount(), 1, 0, 0, 0); });
	else
		RenderContext::record([&](CommandBuffer& cb) { cb.draw(vertex_buffers.front()->getCount(), 1, 0, 0); });
}