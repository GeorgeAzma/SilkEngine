#include "command_pool.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

CommandPool::CommandPool(VkCommandPoolCreateFlags flags, std::optional<uint32_t> queue_family_index)
{
	VkCommandPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = queue_family_index  ? *queue_family_index : Graphics::physical_device->getGraphicsQueue();
	ci.flags = flags;	
	command_pool = Graphics::logical_device->createCommandPool(ci);
}

CommandPool::~CommandPool()
{
	Graphics::logical_device->destroyCommandPool(command_pool);
}

VkCommandBuffer CommandPool::allocate(VkCommandBufferLevel level)
{
	++allocated_command_buffer_count;
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = level;
	return Graphics::logical_device->allocateCommandBuffers(alloc_info).front();
}

void CommandPool::deallocate(const VkCommandBuffer& command_buffer)
{
	Graphics::logical_device->freeCommandBuffers(command_pool, { command_buffer });
	--allocated_command_buffer_count;
}

void CommandPool::reset()
{
	Graphics::logical_device->resetCommandPool(command_pool);
}
