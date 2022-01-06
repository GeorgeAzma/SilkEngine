#include "vertex_buffer.h"
#include "graphics.h"

VertexBuffer::VertexBuffer(const void* data, size_t size)
{
	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	Graphics::vulkanAssert(vkCreateBuffer(*Graphics::logical_device, &buffer_info, nullptr, &vertex_buffer));

	VkMemoryRequirements memory_requirements{};
	vkGetBufferMemoryRequirements(*Graphics::logical_device, vertex_buffer, &memory_requirements);

	VkMemoryAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation_info.allocationSize = memory_requirements.size;
	allocation_info.memoryTypeIndex = findMemoryType(
		memory_requirements.memoryTypeBits, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	Graphics::vulkanAssert(vkAllocateMemory(*Graphics::logical_device, &allocation_info, nullptr, &memory));

	vkBindBufferMemory(*Graphics::logical_device, vertex_buffer, memory, 0);
	
	void* buffer_data;
	vkMapMemory(*Graphics::logical_device, memory, 0, buffer_info.size, 0, &buffer_data);
	std::memcpy(buffer_data, data, buffer_info.size);
	vkUnmapMemory(*Graphics::logical_device, memory);
}

VertexBuffer::~VertexBuffer()
{
	vkDestroyBuffer(*Graphics::logical_device, vertex_buffer, nullptr);
	vkFreeMemory(*Graphics::logical_device, memory, nullptr);
}

uint32_t VertexBuffer::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
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

	VE_CRITICAL("Vulkan: Couldn't find suitable memory type");
}
