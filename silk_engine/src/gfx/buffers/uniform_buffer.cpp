#include "uniform_buffer.h"

UniformBuffer::UniformBuffer(VkDeviceSize size)
	: Buffer(size, 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU)
{
}
