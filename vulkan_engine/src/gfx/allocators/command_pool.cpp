#include "command_pool.h"
#include "gfx/graphics.h"

CommandPool::CommandPool()
{
	auto queue_family_indices = Graphics::physical_device->getQueueFamilyIndices();
	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = *queue_family_indices.graphics;
	create_info.flags =
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | 
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	Graphics::vulkanAssert(vkCreateCommandPool(*Graphics::logical_device, &create_info, nullptr, &command_pool));
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(*Graphics::logical_device, command_pool, nullptr);
}
