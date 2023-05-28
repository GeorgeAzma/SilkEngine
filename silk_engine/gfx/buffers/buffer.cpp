#include "buffer.h"
#include "silk_engine/gfx/allocators/allocator.h"
#include "silk_engine/gfx/render_context.h"
#include "command_buffer.h"

Buffer::Buffer(VkDeviceSize size, Usage usage, const Allocation::Props& allocation_props) :
	ci(VkBufferCreateInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = (VkBufferUsageFlags)usage, .sharingMode = VK_SHARING_MODE_EXCLUSIVE }),
	alloc_ci(VmaAllocationCreateInfo{ .flags = allocation_props.flags, .usage = (VmaMemoryUsage)allocation_props.preferred_device, .priority = allocation_props.priority }),
	allocation(RenderContext::getAllocator().allocateBuffer(ci, alloc_ci, buffer))
{} 

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

bool Buffer::copy(VkBuffer destination, VkDeviceSize size, VkDeviceSize offset, VkDeviceSize dst_offset) const
{
	if (size + offset > ci.size)
		return false;
	copy(destination, buffer, size ? size : ci.size, dst_offset, offset);
	return true;
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
	if (VmaAllocation(allocation) && allocation.isHostVisible())
	{
		allocation.setData(data, size ? size : ci.size, offset);
		return true;
	}
	
	Buffer sb(size ? size : ci.size, Buffer::TRANSFER_SRC, { Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED });
	sb.setData(data);
	sb.copy(buffer, size ? size : ci.size, offset);
	RenderContext::executeTransfer();
	return true;
}

void Buffer::getData(void* data, VkDeviceSize size) const
{
	if (VmaAllocation(allocation) && allocation.isHostVisible())
		allocation.getData(data, size ? size : ci.size);
	else
	{
		Buffer sb(size ? size : ci.size, Buffer::TRANSFER_DST, { Allocation::RANDOM_ACCESS | Allocation::MAPPED });
		copy(sb, sb.getSize());
		RenderContext::executeTransfer();
		sb.getData(data);
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