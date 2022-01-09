#include "buffer.h"
#include "gfx/graphics.h"

Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	: size{size}
{
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	Graphics::vulkanAssert(vkCreateBuffer(*Graphics::logical_device, &buffer_info, nullptr, &buffer));

	VkMemoryRequirements memory_requirements{};
	vkGetBufferMemoryRequirements(*Graphics::logical_device, buffer, &memory_requirements);

	VkMemoryAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation_info.allocationSize = memory_requirements.size;
	allocation_info.memoryTypeIndex = findMemoryType(memory_requirements.memoryTypeBits, properties);

	Graphics::vulkanAssert(vkAllocateMemory(*Graphics::logical_device, &allocation_info, nullptr, &memory));

	Graphics::vulkanAssert(vkBindBufferMemory(*Graphics::logical_device, buffer, memory, 0));
}

Buffer::~Buffer()
{
	vkDestroyBuffer(*Graphics::logical_device, buffer, nullptr);
	vkFreeMemory(*Graphics::logical_device, memory, nullptr);
}

void Buffer::setData(const void* data)
{
	Buffer::setData(data, size, memory);
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

	VE_CORE_ERROR("Vulkan: Couldn't find suitable memory type");
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

	command_buffer.begin(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);

	command_buffer.end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &(const VkCommandBuffer&)command_buffer;

	Graphics::vulkanAssert(vkQueueSubmit(Graphics::logical_device->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE));
	Graphics::vulkanAssert(vkQueueWaitIdle(Graphics::logical_device->getGraphicsQueue()));
}

void Buffer::setData(const void* data, size_t size, VkDeviceMemory memory)
{
	void* buffer_data;
	Graphics::vulkanAssert(vkMapMemory(*Graphics::logical_device, memory, 0, size, 0, &buffer_data));
	std::memcpy(buffer_data, data, size);
	vkUnmapMemory(*Graphics::logical_device, memory);
}
