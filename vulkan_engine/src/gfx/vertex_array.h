#pragma once

#include "buffers/vertex_buffer.h"
#include "buffers/index_buffer.h"

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	VertexArray& addVertexBuffer(std::shared_ptr<VertexBuffer> vertex_buffer);
	VertexArray& setIndexBuffer(std::shared_ptr<IndexBuffer> index_buffer);

	std::shared_ptr<VertexBuffer> getVertexBuffer(size_t index) const { return vertex_buffers[index]; }
	std::shared_ptr<IndexBuffer> getIndexBuffer() const { return index_buffer; }

	void bind();

private:
	std::vector<std::shared_ptr<VertexBuffer>> vertex_buffers = {};
	std::shared_ptr<IndexBuffer> index_buffer = nullptr;
};