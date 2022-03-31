#pragma once

#include "buffer.h"

class IndirectBuffer : public Buffer
{
public:
	IndirectBuffer(VkDeviceSize size);
};