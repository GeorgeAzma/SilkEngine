#include "staging_buffer.h"
#include "command_buffer.h"
#include "gfx/graphics.h"
#include "buffer_utils.h"

StagingBuffer::StagingBuffer(const void* data, VkDeviceSize size)
    : Buffer(size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
    setData(data);
}

void StagingBuffer::copy(VkBuffer destination) const //~0.8ms
{
    BufferUtils::copy(destination, buffer, size);
}
