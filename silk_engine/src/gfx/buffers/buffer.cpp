#include "buffer.h"
#include "staging_buffer.h"
#include "gfx/allocators/allocator.h"
#include "gfx/allocators/command_pool.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"

Buffer::Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage vma_usage)
	: size(size), needs_staging(EnumInfo::needsStaging(vma_usage))
{
	ci.size = size;
	ci.usage = usage;
	ci.sharingMode = vk::SharingMode::eExclusive;
	allocation_create_info.usage = vma_usage;	
	const VkBufferCreateInfo& vk_ci = (const VkBufferCreateInfo&)ci;
	VkBuffer& vk_buffer = (VkBuffer&)buffer;
	Graphics::vulkanAssert(vk::Result(vmaCreateBuffer(*Graphics::allocator, &vk_ci, &allocation_create_info, &vk_buffer, &allocation, nullptr)));
} 

Buffer::~Buffer()
{
	delete[] data;
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
}

void Buffer::resize(vk::DeviceSize size)
{
	if (size == this->size)
		return;
	this->size = size;
	delete[] data;
	data = nullptr;
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
	ci.size = size;
	const VkBufferCreateInfo& vk_ci = (const VkBufferCreateInfo&)ci;
	VkBuffer& vk_buffer = (VkBuffer&)buffer;
	Graphics::vulkanAssert(vk::Result(vmaCreateBuffer(*Graphics::allocator, &vk_ci, &allocation_create_info, &vk_buffer, &allocation, nullptr)));
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
	if (!data)
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

void Buffer::setDataChecked(const void* data, size_t size, size_t offset)
{
	SK_ASSERT(((size ? size : this->size) + offset) <= this->size,
		"Vulkan: Can't map memory, it's out of bounds");

	if (!this->data)
		this->data = new uint8_t[this->size];

	if (std::memcmp(data, this->data + offset, size) != 0)
	{
		std::memcpy(this->data + offset, data, size);
		setData(data, size, offset);
	}
}

void Buffer::getData(void* data, size_t size) const
{
	if (needs_staging)
	{
		StagingBuffer sb(nullptr, size ? size : this->size);
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

void Buffer::insertMemoryBarrier(const vk::Buffer& buffer, vk::AccessFlags source_access_mask, vk::AccessFlags destination_access_mask, vk::PipelineStageFlags source_stage_mask, vk::PipelineStageFlags destination_stage_mask, vk::DeviceSize offset, vk::DeviceSize size)
{
	vk::BufferMemoryBarrier barrier = {};
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;
	Graphics::active.command_buffer.pipelineBarrier(source_stage_mask, destination_stage_mask, vk::DependencyFlags(0), {}, {barrier}, {});
}

void Buffer::copy(vk::Buffer destination, vk::Buffer source, vk::DeviceSize size, vk::DeviceSize dst_offset, vk::DeviceSize src_offset)
{
	CommandBuffer command_buffer;
	command_buffer.begin();

	vk::BufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;
	vk::CommandBuffer(command_buffer).copyBuffer(source, destination, { copy_region });

	command_buffer.submitIdle();
}