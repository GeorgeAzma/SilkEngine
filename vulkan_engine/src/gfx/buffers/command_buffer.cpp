#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/command_pool.h"

CommandBuffer::CommandBuffer(size_t count, VkCommandBufferLevel level)
	: level(level)
{
	command_buffers.resize(count);
	recorded.resize(count, false);

	VkCommandBufferAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = level;
	allocation_info.commandPool = *Graphics::command_pool; //Creating seperate command pool with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT might be more efficient
	allocation_info.commandBufferCount = command_buffers.size();

	Graphics::vulkanAssert(vkAllocateCommandBuffers(*Graphics::logical_device, &allocation_info, command_buffers.data()));
}

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(*Graphics::logical_device, *Graphics::command_pool, command_buffers.size(), command_buffers.data());
}

void CommandBuffer::begin(VkCommandBufferUsageFlagBits usage, size_t index)
{
	if (Graphics::active.command_buffer == command_buffers[index])
		return;

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;

	if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		VkCommandBufferInheritanceInfo inheritance_info{};
		inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		
		if ((usage & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT) == VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
		{
			inheritance_info.renderPass = Graphics::active.render_pass;
			inheritance_info.subpass = Graphics::active.subpass;
		}

		begin_info.pInheritanceInfo = &inheritance_info;
	}

	Graphics::vulkanAssert(vkBeginCommandBuffer(command_buffers[index], &begin_info));

	Graphics::active.command_buffer = command_buffers[index];
}

void CommandBuffer::end(size_t index)
{
	if (Graphics::active.command_buffer != command_buffers[index])
		return;

	Graphics::vulkanAssert(vkEndCommandBuffer(command_buffers[index]));
	
	recorded[index] = true;
	Graphics::active.command_buffer = nullptr;
}

void CommandBuffer::submit(const CommandBufferSubmitInfo& command_buffer_submit_info)
{
	end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submit_info.waitSemaphoreCount = command_buffer_submit_info.wait_semaphores.size();
	submit_info.pWaitSemaphores = command_buffer_submit_info.wait_semaphores.data();

	submit_info.pWaitDstStageMask = command_buffer_submit_info.wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[command_buffer_submit_info.index];

	submit_info.signalSemaphoreCount = command_buffer_submit_info.signal_semaphores.size();
	submit_info.pSignalSemaphores = command_buffer_submit_info.signal_semaphores.data();

	fence = command_buffer_submit_info.fence;

	if (fence != VK_NULL_HANDLE)
	{
		Graphics::vulkanAssert(vkResetFences(*Graphics::logical_device, 1, &fence));
	}

	Graphics::vulkanAssert(vkQueueSubmit(command_buffer_submit_info.queue != VK_NULL_HANDLE ? command_buffer_submit_info.queue : Graphics::logical_device->getGraphicsQueue(), 1, &submit_info, fence));
}

void CommandBuffer::wait()
{
	if (fence != VK_NULL_HANDLE)
	{
		Graphics::vulkanAssert(vkWaitForFences(*Graphics::logical_device, 1, &fence, VK_TRUE, UINT64_MAX));
	}
	else
	{
		Graphics::vulkanAssert(vkQueueWaitIdle(queue != VK_NULL_HANDLE ? queue : Graphics::logical_device->getGraphicsQueue()));
	}
}
