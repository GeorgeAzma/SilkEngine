#include "uniform_buffer.h"

UniformBuffer::UniformBuffer(vk::DeviceSize size)
	: Buffer(size, 
		vk::BufferUsageFlagBits::eUniformBuffer,
		VMA_MEMORY_USAGE_CPU_TO_GPU)
{
}
