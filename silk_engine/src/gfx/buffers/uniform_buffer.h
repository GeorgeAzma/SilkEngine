#pragma once

#include "buffer.h"

class UniformBuffer : public Buffer
{
public:
	UniformBuffer(vk::DeviceSize size);
};