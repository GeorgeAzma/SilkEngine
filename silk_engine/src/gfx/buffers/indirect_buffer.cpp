#include "indirect_buffer.h"
#include "staging_buffer.h"

IndirectBuffer::IndirectBuffer(vk::DeviceSize size)
	: Buffer(size, 
		vk::BufferUsageFlagBits::eStorageBuffer | 
		vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eIndirectBuffer,
		VMA_MEMORY_USAGE_CPU_TO_GPU)
{
}