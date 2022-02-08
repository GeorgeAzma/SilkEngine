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
		vmaMapMemory(*Graphics::allocator, allocation, &buffer_data);
		std::memcpy((uint8_t*)buffer_data + offset, data, size ? size : this->size);
		vmaUnmapMemory(*Graphics::allocator, allocation);
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
	vmaMapMemory(*Graphics::allocator, allocation, &buffer_data);
	std::memcpy(data, buffer_data, size ? size : this->size);
	vmaUnmapMemory(*Graphics::allocator, allocation);
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

void Buffer::copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset, VkDeviceSize src_offset)
{
	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;
	vkCmdCopyBuffer(Graphics::active.command_buffer, source, destination, 1, &copy_region);
	
	command_buffer.end();
	command_buffer.submitIdle();
}