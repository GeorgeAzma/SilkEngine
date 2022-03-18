#pragma once

#include "buffer.h"

class StagingBuffer : public Buffer
{
public:
	StagingBuffer(const void* data, vk::DeviceSize size, bool transfer_destination = false);
	void copy(vk::Buffer destination, vk::DeviceSize offset = 0) const;
};