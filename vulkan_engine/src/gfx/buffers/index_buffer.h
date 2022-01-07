#pragma once

#include "buffer.h"
#include "gfx/enums.h"

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(const void* data, VkDeviceSize count, IndexType index_type = IndexType::UINT32);

	void bind();

private:
	VkIndexType index_type;
};