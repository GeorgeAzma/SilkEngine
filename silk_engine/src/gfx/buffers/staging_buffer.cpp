#include "staging_buffer.h"
#include "command_buffer.h"
#include "gfx/graphics.h"

StagingBuffer::StagingBuffer(const void* data, vk::DeviceSize size, bool transfer_destination)
    : Buffer(size,
             vk::BufferUsageFlagBits::eTransferSrc | (transfer_destination ? vk::BufferUsageFlagBits::eTransferDst : vk::BufferUsageFlagBits(0)),
             VMA_MEMORY_USAGE_CPU_ONLY)
{
    setData(data, size);
}

void StagingBuffer::copy(vk::Buffer destination, vk::DeviceSize offset) const
{
    Buffer::copy(destination, buffer, size, offset);
}
