#include "buffer.h"
#include "staging_buffer.h"
#include "gfx/allocators/allocator.h"
#include "gfx/allocators/command_pool.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/graphics.h"

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage)
	: size(size), needs_staging(vma_usage == VMA_MEMORY_USAGE_GPU_ONLY)
{
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocation_create_info = {};
	allocation_create_info.usage = vma_usage;
	
	Graphics::vulkanAssert(vmaCreateBuffer(*Graphics::allocator, &buffer_info, &allocation_create_info, &buffer, &allocation, nullptr));
}

Buffer::~Buffer()
{
	delete[] data;
	vmaDestroyBuffer(*Graphics::allocator, buffer, allocation);
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
	void* buffer_data;
	map(&buffer_data);
	std::memcpy(data, buffer_data, size ? size : this->size);
	unmap();
}

uint32_t Buffer::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(*Graphics::physical_device, &memory_properties);

	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
	{
		if ((type_filter & (1 << i)) &&
			(memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	SK_ERROR("Vulkan: Couldn't find suitable memory type");
	return 0;
}

void Buffer::insertMemoryBarrier(const VkBuffer& buffer, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDeviceSize offset, VkDeviceSize size)
{
	VkBufferMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.buffer = buffer;
	barrier.offset = offset;
	barrier.size = size;
	vkCmdPipelineBarrier(Graphics::active.command_buffer, source_stage_mask, destination_stage_mask, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

void Buffer::copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset, VkDeviceSize src_offset)
{
	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;
	vkCmdCopyBuffer(Graphics::active.command_buffer, source, destination, 1, &copy_region);
	
	command_buffer.submitIdle();
}