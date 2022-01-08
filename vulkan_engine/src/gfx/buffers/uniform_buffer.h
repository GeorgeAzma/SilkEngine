#pragma once

#include "buffer.h"

class UniformBuffer : public Buffer //includes setData(const void* data)
{
public:
	UniformBuffer(VkDeviceSize size);
};