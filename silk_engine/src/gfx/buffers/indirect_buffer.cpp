#include "indirect_buffer.h"
#include "staging_buffer.h"

IndirectBuffer::IndirectBuffer(VkDeviceSize size)
	: Buffer(size, 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, 
		VMA_MEMORY_USAGE_CPU_TO_GPU)
{
}