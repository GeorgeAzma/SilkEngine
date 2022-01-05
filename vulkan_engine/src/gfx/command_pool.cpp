#include "command_pool.h"
#include "graphics.h"

CommandPool::CommandPool()
{
	auto queue_family_indices = Graphics::getPhysicalDevice()->getQueueFamilyIndices();
	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = *queue_family_indices.graphics;

	VE_CORE_ASSERT(vkCreateCommandPool(Graphics::getLogicalDevice()->getLogicalDevice(), &create_info, nullptr, &command_pool) == VK_SUCCESS,
		"Vulkan: Couldn't create a command pool");
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(Graphics::getLogicalDevice()->getLogicalDevice(), command_pool, nullptr);
}
