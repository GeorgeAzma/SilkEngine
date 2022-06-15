#include "buffer.h"
#include "staging_buffer.h"
#include "gfx/allocators/allocator.h"
#include "gfx/allocators/command_pool.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage)
	: size(size), needs_staging(Allocator::needsStaging(vma_usage))
{
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.size = size;
	ci.usage = usage;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	allocation_ci.usage = vma_usage;
	Graphics::vulkanAssert(vmaCreateBuffer(*Graphics::allocator, &ci, &allocation_ci, &buffer, &allocation, nullptr));
	SK_TRACE("Buffer created with size: {}", size);
} 

Buffer::~Buffer()
{
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
	SK_TRACE("Buffer destroyed: {}", size);
}

void Buffer::resize(VkDeviceSize size)
{
	if (size == this->size)
		return;
	this->size = size;
	ci.size = size;
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
	Graphics::vulkanAssert(vmaCreateBuffer(*Graphics::allocator, &ci, &allocation_ci, &buffer, &allocation, nullptr));
	SK_TRACE("Buffer resized: {}", size);
}

void Buffer::map(void** data) const
{
	if (mapped)
		return;
	SK_ASSERT(!needs_staging, "Can't map the buffer if it's usage is set to GPU_ONLY");
	vmaMapMemory(*Graphics::allocator, allocation, data);
	mapped = true;
}
 
void Buffer::unmap() const
{
	if (!mapped)
		return;
	SK_ASSERT(!needs_staging, "Can't unmap the buffer if it's usage is set to GPU_ONLY");
	vmaUnmapMemory(*Graphics::allocator, allocation);
	mapped = false;
}

void Buffer::setData(const void* data, size_t size, size_t offset)
{
	if (!data || !size)
		return;

	SK_ASSERT(((size ? size : this->size) + offset) <= this->size, 
		"Vulkan: Can't map memory, it's out of bounds");

	if (needs_staging)
	{
		StagingBuffer sb(data, size);
		sb.copy(buffer, offset);
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy((uint8_t*)buffer_data + offset, data, size ? size : this->size);
		unmap();
	}
}

void Buffer::getData(void* data, size_t size) const
{
	if (needs_staging)
	{
		StagingBuffer sb(nullptr, size ? size : this->size, true);
		copy(sb, buffer, sb.size, 0, 0);
		sb.getData(data);
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy(data, buffer_data, size ? size : this->size);
		unmap();
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
	Graphics::getActiveCommandBuffer().pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), {}, {barrier}, {});
}

void Buffer::copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset, VkDeviceSize src_offset)
{
	CommandBuffer command_buffer;
	command_buffer.begin();

	VkBufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;
	Graphics::getActiveCommandBuffer().copyBuffer(source, destination, { copy_region });

	command_buffer.submitIdle();
}