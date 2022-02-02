#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/command_pool.h"

CommandBuffer::CommandBuffer(VkCommandBufferLevel level, VkQueueFlagBits queue_type)
	: level(level), queue_type(queue_type)
{
	VkCommandBufferAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = level;
	allocation_info.commandPool = *Graphics::command_pool; //Creating seperate command pool with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT might be more efficient
	allocation_info.commandBufferCount = 1;

	Graphics::vulkanAssert(vkAllocateCommandBuffers(*Graphics::logical_device, &allocation_info, &command_buffer));
}

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(*Graphics::logical_device, *Graphics::command_pool, 1, &command_buffer);
}

void CommandBuffer::begin(VkCommandBufferUsageFlagBits usage)
{
	if (Graphics::active.command_buffer == command_buffer)
		return;
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;

	Graphics::vulkanAssert(vkBeginCommandBuffer(command_buffer, &begin_info));
	Graphics::active.command_buffer = command_buffer;
}

void CommandBuffer::end()
{
	if (Graphics::active.command_buffer != command_buffer)
		return;

	Graphics::vulkanAssert(vkEndCommandBuffer(command_buffer)); 
	Graphics::active.command_buffer = VK_NULL_HANDLE;
}

void CommandBuffer::submit(const CommandBufferSubmitInfo& command_buffer_submit_info)
{
	end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	submit_info.pWaitDstStageMask = command_buffer_submit_info.wait_stages;
	submit_info.waitSemaphoreCount = command_buffer_submit_info.wait_semaphores.size();
	submit_info.pWaitSemaphores = command_buffer_submit_info.wait_semaphores.data();

	submit_info.signalSemaphoreCount = command_buffer_submit_info.signal_semaphores.size();
	submit_info.pSignalSemaphores = command_buffer_submit_info.signal_semaphores.data();

	if (command_buffer_submit_info.fence != VK_NULL_HANDLE)
		Graphics::vulkanAssert(vkResetFences(*Graphics::logical_device, 1, &command_buffer_submit_info.fence));
	
	Graphics::vulkanAssert(vkQueueSubmit(getQueue(), 1, &submit_info, command_buffer_submit_info.fence));
}

void CommandBuffer::submitIdle()
{
	end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkFence fence;
	Graphics::vulkanAssert(vkCreateFence(*Graphics::logical_device, &fence_info, nullptr, &fence));
	Graphics::vulkanAssert(vkResetFences(*Graphics::logical_device, 1, &fence));

	Graphics::vulkanAssert(vkQueueSubmit(getQueue(), 1, &submit_info, fence));
	Graphics::vulkanAssert(vkWaitForFences(*Graphics::logical_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));

	vkDestroyFence(*Graphics::logical_device, fence, nullptr);
}

VkQueue CommandBuffer::getQueue() const
{
	switch (queue_type) 
	{
	case VK_QUEUE_GRAPHICS_BIT:
		return Graphics::logical_device->getGraphicsQueue(); 
	case VK_QUEUE_TRANSFER_BIT:
		return Graphics::logical_device->getTransferQueue();
	case VK_QUEUE_COMPUTE_BIT:
		return Graphics::logical_device->getComputeQueue();
	default:
		return Graphics::logical_device->getGraphicsQueue(); //Or nullptr
	}
}
