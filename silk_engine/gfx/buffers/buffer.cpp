#include "buffer.h"
#include "gfx/allocators/allocator.h"
#include "gfx/graphics.h"
#include "gfx/queues/command_queue.h"
#include "command_buffer.h"

Buffer::Buffer(VkDeviceSize size, Usage usage, const Allocation::Props& allocation_props)
{
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.size = size;
	ci.usage = (VkBufferUsageFlags)usage;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	allocation_ci.usage = (VmaMemoryUsage)allocation_props.prefer_device;
	allocation_ci.flags = allocation_props.flags;
	Graphics::vulkanAssert(vmaCreateBuffer(*Graphics::allocator, &ci, &allocation_ci, &buffer, &(VmaAllocation&)allocation, nullptr));
} 

Buffer::~Buffer()
{
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
}

void Buffer::resize(VkDeviceSize size)
{
	if (size == ci.size)
		return;
	ci.size = size;
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
	Graphics::vulkanAssert(vmaCreateBuffer(*Graphics::allocator, &ci, &allocation_ci, &buffer, &(VmaAllocation&)allocation, nullptr));
}

void Buffer::copy(VkBuffer destination, VkDeviceSize size, VkDeviceSize offset, VkDeviceSize dst_offset) const
{
	copy(destination, buffer, size, dst_offset, offset);
}

void Buffer::setData(const void* data, size_t size, size_t offset)
{
	if (!data)
		return;
	if (allocation.isHostVisible())
		allocation.setData(data, size ? size : ci.size, offset);
	else
	{
		Buffer sb(size ? size : ci.size, Buffer::TRANSFER_SRC, { Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED });
		sb.setData(data);
		sb.copy(buffer, size ? size : ci.size, offset);
	}
}

void Buffer::getData(void* data, size_t size) const
{
	if (allocation.isHostVisible())
		allocation.getData(data, size ? size : ci.size);
	else
	{
		Buffer sb(size ? size : ci.size, Buffer::TRANSFER_DST, { Allocation::RANDOM_ACCESS | Allocation::MAPPED });
		sb.getData(data);
	}
}

void Buffer::insertMemoryBarrier(const VkBuffer& buffer, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDeviceSize offset, VkDeviceSize size)
{
	VkBufferMemoryBarrier barrier = {};
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;
	Graphics::submit([&] (CommandBuffer& cb) { cb.pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), {}, { barrier }, {}); });
}

void Buffer::copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset, VkDeviceSize src_offset)
{
	Graphics::submit([&] (CommandBuffer& cb)
		{
			VkBufferCopy copy_region{};
			copy_region.srcOffset = src_offset;
			copy_region.dstOffset = dst_offset;
			copy_region.size = size;
			cb.copyBuffer(source, destination, { copy_region });
		});
	Graphics::execute();
}