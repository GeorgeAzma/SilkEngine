#include "command_pool.h"
#include "gfx/graphics.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"

CommandPool::CommandPool(vk::CommandPoolCreateFlags flags, std::optional<uint32_t> queue_family_index)
{
	vk::CommandPoolCreateInfo ci;
	ci.queueFamilyIndex = queue_family_index  ? *queue_family_index : *Graphics::physical_device->getQueueFamilyIndices().graphics;
	ci.flags = flags;	
	Graphics::logical_device->createCommandPool(ci);
}

CommandPool::~CommandPool()
{
	Graphics::logical_device->destroyCommandPool(command_pool);
}