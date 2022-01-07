#pragma once
#include "buffer.h"
#include "staging_buffer.h"

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(const void* data, VkDeviceSize size);

	void bind();
};