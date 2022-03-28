#include "command_pool.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

CommandPool::CommandPool(vk::CommandPoolCreateFlags flags, std::optional<uint32_t> queue_family_index)
{
	vk::CommandPoolCreateInfo ci;
	ci.queueFamilyIndex = queue_family_index  ? *queue_family_index : *Graphics::physical_device->getQueueFamilyIndices().graphics;
	ci.flags = flags;	
	command_pool = Graphics::logical_device->createCommandPool(ci);
}

CommandPool::~CommandPool()
{
	SK_ASSERT(allocated_command_buffer_count == 0, "Called command pool destructor when command pool still contained command buffers");
	Graphics::logical_device->destroyCommandPool(command_pool);
}

vk::CommandBuffer CommandPool::allocate(vk::CommandBufferLevel level)
{
	++allocated_command_buffer_count;
	return Graphics::logical_device->allocateCommandBuffers({ command_pool, level, 1 }).front();
}

void CommandPool::deallocate(const vk::CommandBuffer& command_buffer)
{
	SK_ASSERT(allocated_command_buffer_count > 0, "Can't deallocate pool's command buffer when it doesn't have any");
	Graphics::logical_device->freeCommandBuffers(command_pool, { command_buffer });
	--allocated_command_buffer_count;
}