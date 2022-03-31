#pragma once

#include "buffer.h"

class StagingBuffer : public Buffer
{
public:
	StagingBuffer(const void* data, VkDeviceSize size, bool transfer_destination = false);
	void copy(VkBuffer destination, VkDeviceSize offset = 0) const;
};