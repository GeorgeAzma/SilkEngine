#pragma once

#include "buffer.h"

class StagingBuffer : public Buffer
{
public:
	StagingBuffer(const void* data, VkDeviceSize size);
	void copy(VkBuffer destination) const;
};