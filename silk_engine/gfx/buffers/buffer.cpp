#include "buffer.h"
#include "silk_engine/gfx/allocators/allocator.h"
#include "silk_engine/gfx/render_context.h"
#include "command_buffer.h"

Buffer::Buffer(VkDeviceSize size, Usage usage, const Allocation::Props& allocation_props) :
	ci(VkBufferCreateInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = (VkBufferUsageFlags)usage, .sharingMode = VK_SHARING_MODE_EXCLUSIVE }),
	alloc_ci(VmaAllocationCreateInfo{ .flags = allocation_props.flags, .usage = (VmaMemoryUsage)allocation_props.preferred_device, .priority = allocation_props.priority }),
	allocation(RenderContext::getAllocator().allocateBuffer(ci, alloc_ci, buffer))
{
	SK_ASSERT(size > 0, "Buffer size can't be 0");
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

void Buffer::reallocate(VkDeviceSize size)
{
	if (size == ci.size)
		return;
	std::vector<uint8_t> data(std::min(size, ci.size));
	getData(data.data(), data.size());
	resize(size);
	setData(data.data(), data.size());
}

void Buffer::copy(VkBuffer destination, VkDeviceSize size, VkDeviceSize offset, VkDeviceSize dst_offset) const
{
	copy(destination, buffer, size ? size : (ci.size - offset), dst_offset, offset);
}

void Buffer::bindVertex(uint32_t first, VkDeviceSize offset)
{
	RenderContext::getCommandBuffer().bindVertexBuffers(first, { buffer }, { offset });
}

void Buffer::bindIndex(VkIndexType index_type, VkDeviceSize offset)
{
	RenderContext::getCommandBuffer().bindIndexBuffer(buffer, offset, index_type);
}

void Buffer::drawIndirect(uint32_t index)
{
	RenderContext::getCommandBuffer().drawIndirect(buffer, index * sizeof(VkDrawIndirectCommand), 1, sizeof(VkDrawIndirectCommand));
}

void Buffer::drawIndexedIndirect(uint32_t index)
{
	RenderContext::getCommandBuffer().drawIndexedIndirect(buffer, index * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
}

bool Buffer::setData(const void* data, VkDeviceSize size, VkDeviceSize offset)
{
	if (size + offset > ci.size)
		return false;
	VkDeviceSize max_size = size ? size : (ci.size - offset);
	if (VmaAllocation(allocation) && allocation.isHostVisible())
	{
		allocation.setData(data, max_size, offset);
		return true;
	}

	Buffer sb(max_size, Buffer::TRANSFER_SRC, { Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED });
	sb.setData(data, max_size);
	sb.copy(buffer, max_size, offset);
	RenderContext::executeTransfer();
	return true;
}

void Buffer::getData(void* data, VkDeviceSize size, VkDeviceSize offset) const
{
	getDataRanges({ { data, size, offset } });
}

void Buffer::getDataRanges(const std::vector<Range>& ranges) const
{
	if (VmaAllocation(allocation) && allocation.isHostVisible())
		for (const auto& range : ranges)
			allocation.getData(range.data, range.size ? range.size : (ci.size - range.offset), range.offset);
	else
	{
		VkDeviceSize max_size = 0;
		for (const auto& range : ranges)
			max_size = max(max_size, range.size ? range.size + range.offset : ci.size);
		Buffer sb(max_size, Buffer::TRANSFER_DST, { Allocation::RANDOM_ACCESS | Allocation::MAPPED });
		copy(sb, max_size);
		RenderContext::executeTransfer();
		sb.getDataRanges(ranges);
	}
}

void Buffer::insertMemoryBarrier(VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDeviceSize offset, VkDeviceSize size) const
{
	insertMemoryBarrier(buffer, source_access_mask, destination_access_mask, source_stage_mask, destination_stage_mask, offset, size ? size : ci.size);
}

void Buffer::insertMemoryBarrier(const VkBuffer& buffer, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDeviceSize offset, VkDeviceSize size)
{
	VkBufferMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;
	RenderContext::getCommandBuffer().pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), {}, { barrier }, {});
}

void Buffer::copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset, VkDeviceSize src_offset)
{
	VkBufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;
	RenderContext::getTransferCommandBuffer().copyBuffer(source, destination, { copy_region });
}