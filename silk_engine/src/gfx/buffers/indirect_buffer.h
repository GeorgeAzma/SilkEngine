#pragma once

#include "buffer.h"

class IndirectBuffer : public Buffer
{
public:
	IndirectBuffer(vk::DeviceSize size);
};