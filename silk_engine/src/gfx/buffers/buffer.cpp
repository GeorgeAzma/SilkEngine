#include "buffer.h"
#include "gfx/allocators/allocator.h"
#include "gfx/allocators/command_pool.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage)
	: size(size)
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

	void* buffer_data;
	vmaMapMemory(*Graphics::allocator, allocation, &buffer_data);
	std::memcpy((uint8_t*)buffer_data + offset, data, size ? size : this->size);
	vmaUnmapMemory(*Graphics::allocator, allocation);
}

void Buffer::setDataChecked(const void* data, size_t size, size_t offset)
{
	SK_ASSERT(((size ? size : this->size) + offset) <= this->size,
		"Vulkan: Can't map memory, it's out of bounds");

	if (!this->data)
	{
		this->data = new uint8_t[this->size];
	}

	if (std::memcmp(data, this->data + offset, size) != 0)
	{
		std::memcpy(this->data + offset, data, size);
		setData(data, size, offset);
	}
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

void Buffer::copy(VkBuffer destination, VkBuffer source, size_t size)
{
	VkCommandBufferAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation_info.commandPool = *Graphics::command_pool; //Creating seperate command pool with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT might be more efficient
	allocation_info.commandBufferCount = 1;

	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);

	command_buffer.end();
	command_buffer.submit();
	command_buffer.wait();
}