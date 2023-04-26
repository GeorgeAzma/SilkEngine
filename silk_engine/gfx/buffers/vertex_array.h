#pragma once

#include "vertex_buffer.h"
#include "index_buffer.h"

class VertexArray
{
public:
	~VertexArray();

	VertexArray& addVertexBuffer(const shared<VertexBuffer>& vertex_buffer);
	VertexArray& setIndexBuffer(const shared<IndexBuffer>& index_buffer);

	const shared<VertexBuffer>& getVertexBuffer(size_t index) const { return vertex_buffers[index]; }
	const shared<IndexBuffer>& getIndexBuffer() const { return index_buffer; }

	bool hasIndexBuffer() const { return index_buffer != nullptr; }

	void bind(const std::vector<VkDeviceSize>& offsets = {}, VkDeviceSize index_offset = 0) const;
	void draw() const;

private:
	std::vector<shared<VertexBuffer>> vertex_buffers = {};
	std::vector<VkBuffer> vk_vertex_buffers = {};
	shared<IndexBuffer> index_buffer = nullptr;
};