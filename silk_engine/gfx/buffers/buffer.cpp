#include "buffer.h"
#include "gfx/allocators/allocator.h"
#include "gfx/render_context.h"
#include "command_buffer.h"

Buffer::Buffer(VkDeviceSize size, Usage usage, const Allocation::Props& allocation_props)
{
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.size = size;
	ci.usage = (VkBufferUsageFlags)usage;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	alloc_ci.usage = (VmaMemoryUsage)allocation_props.preferred_device;
	alloc_ci.flags = allocation_props.flags;
	alloc_ci.priority = allocation_props.priority;
	alloc_ci.priority += 0.1f * ((usage & UsageBits::VERTEX) > 0);
	alloc_ci.priority += 0.1f * ((usage & UsageBits::INDEX) > 0);
	alloc_ci.priority += 0.1f * ((usage & UsageBits::STORAGE) > 0);
	allocation = RenderContext::getAllocator().allocateBuffer(ci, alloc_ci, buffer);
} 

Buffer::~Buffer()
{
	RenderContext::getAllocator().destroyBuffer(buffer, allocation);
}

void Buffer::resize(VkDeviceSize size)
{
	if (size == ci.size)
		return;
	ci.size = size;
	RenderContext::getAllocator().destroyBuffer(buffer, allocation);
	allocation = RenderContext::getAllocator().allocateBuffer(ci, alloc_ci, buffer);
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
		RenderContext::executeTransfer();
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
	RenderContext::submit([&] (CommandBuffer& cb) { cb.pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), {}, { barrier }, {}); });
}

void Buffer::copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset, VkDeviceSize src_offset)
{
	RenderContext::submitTransfer([&] (CommandBuffer& cb)
		{
			VkBufferCopy copy_region{};
			copy_region.srcOffset = src_offset;
			copy_region.dstOffset = dst_offset;
			copy_region.size = size;
			cb.copyBuffer(source, destination, { copy_region });
		});
}