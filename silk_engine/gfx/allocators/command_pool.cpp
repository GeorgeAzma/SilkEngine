#include "command_pool.h"
#include "gfx/render_context.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

CommandPool::CommandPool(VkCommandPoolCreateFlags flags, std::optional<uint32_t> queue_family_index)
	: flags(flags)
{
	VkCommandPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = queue_family_index  ? *queue_family_index : RenderContext::getPhysicalDevice().getGraphicsQueue();
	ci.flags = flags;	
	command_pool = RenderContext::getLogicalDevice().createCommandPool(ci);
}

CommandPool::~CommandPool()
{
	RenderContext::getLogicalDevice().destroyCommandPool(command_pool);
}

VkCommandBuffer CommandPool::allocate(VkCommandBufferLevel level)
{
	++allocated_command_buffer_count;
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandBufferCount = 1;
	alloc_info.commandPool = command_pool;
	alloc_info.level = level;
	return RenderContext::getLogicalDevice().allocateCommandBuffers(alloc_info).front();
}

void CommandPool::deallocate(const VkCommandBuffer& command_buffer)
{
	RenderContext::getLogicalDevice().freeCommandBuffers(command_pool, { command_buffer });
	--allocated_command_buffer_count;
}

void CommandPool::reset(bool free)
{
	if (!allocated_command_buffer_count)
		return;
	RenderContext::getLogicalDevice().resetCommandPool(command_pool, free * VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}
