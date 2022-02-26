#pragma once

#include "buffer.h"

class StagingBuffer : public Buffer
{
public:
	StagingBuffer(const void* data, vk::DeviceSize size);
	void copy(vk::Buffer destination, vk::DeviceSize offset = 0) const;
};