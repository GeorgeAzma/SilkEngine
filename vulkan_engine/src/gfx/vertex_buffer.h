#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, size_t size);
	~VertexBuffer();

	operator const VkBuffer& () const { return vertex_buffer; }

private:
	uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

private:
	VkBuffer vertex_buffer;
	VkDeviceMemory memory;
};