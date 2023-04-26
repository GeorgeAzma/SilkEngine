#pragma once

#include "buffer.h"

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(const void* data, VkDeviceSize vertex_size, VkDeviceSize vertex_count, bool instanced = false);

	void bind(size_t binding = 0, VkDeviceSize offset = 0);

	VkDeviceSize getVertexSize() const { return vertex_size; }
	uint32_t getCount() const { return vertex_count; }

private:
	VkDeviceSize vertex_size = 0;
	uint32_t vertex_count = 0;
};