#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

class VertexArray
{
public:
	~VertexArray();

	VertexArray& addVertexBuffer(shared<VertexBuffer> vertex_buffer);
	VertexArray& setIndexBuffer(shared<IndexBuffer> index_buffer);

	shared<VertexBuffer> getVertexBuffer(size_t index) const { return vertex_buffers[index]; }
	shared<IndexBuffer> getIndexBuffer() const { return index_buffer; }

	void bind();

private:
	std::vector<shared<VertexBuffer>> vertex_buffers = {};
	shared<IndexBuffer> index_buffer = nullptr;
};