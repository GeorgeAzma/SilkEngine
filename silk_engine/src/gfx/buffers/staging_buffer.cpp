#include "staging_buffer.h"
#include "command_buffer.h"
#include "gfx/graphics.h"

StagingBuffer::StagingBuffer(const void* data, VkDeviceSize size)
    : Buffer(size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_CPU_ONLY)
{
    setData(data, size);
}

void StagingBuffer::copy(VkBuffer destination, VkDeviceSize offset) const
{
    Buffer::copy(destination, buffer, size, offset);
}
