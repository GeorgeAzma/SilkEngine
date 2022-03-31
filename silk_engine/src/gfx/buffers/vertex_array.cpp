#include "vertex_array.h"
#include "gfx/graphics.h"
#include "gfx/buffers/command_buffer.h"

VertexArray::~VertexArray()
{
	for (auto& vertex_buffer : vertex_buffers)
		vertex_buffer = nullptr;
	index_buffer = nullptr;
}

VertexArray& VertexArray::addVertexBuffer(shared<VertexBuffer> vertex_buffer)
{
	vertex_buffers.emplace_back(vertex_buffer);
	return *this;
}

VertexArray& VertexArray::setIndexBuffer(shared<IndexBuffer> index_buffer)
{
	this->index_buffer = index_buffer;
	return *this;
}

void VertexArray::bind()
{
	std::vector<VkBuffer> buffers(vertex_buffers.size());
	for (size_t i = 0; i < vertex_buffers.size(); ++i)
		buffers[i] = *vertex_buffers[i];
	Graphics::getActiveCommandBuffer().bindVertexBuffers(0, buffers);
	index_buffer->bind();
}
