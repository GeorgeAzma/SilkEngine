#include "command_pool.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

CommandPool::CommandPool(VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = *Graphics::physical_device->getQueueFamilyIndices().graphics;
	create_info.flags = flags;

	Graphics::vulkanAssert(vkCreateCommandPool(*Graphics::logical_device, &create_info, nullptr, &command_pool));
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(*Graphics::logical_device, command_pool, nullptr);
}

void CommandPool::bind() const
{
	Graphics::active.command_pool = command_pool;
}