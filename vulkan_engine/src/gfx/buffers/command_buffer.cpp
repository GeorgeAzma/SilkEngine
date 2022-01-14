#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/graphics_state.h"

CommandBuffer::CommandBuffer(size_t count)
{
	command_buffers.resize(count);

	VkCommandBufferAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation_info.commandPool = *Graphics::command_pool; //Creating seperate command pool with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT might be more efficient
	allocation_info.commandBufferCount = command_buffers.size();

	Graphics::vulkanAssert(vkAllocateCommandBuffers(*Graphics::logical_device, &allocation_info, command_buffers.data()));
}

CommandBuffer::~CommandBuffer()
{
	if (graphics_state.command_buffer) 
		end();

	vkFreeCommandBuffers(*Graphics::logical_device, *Graphics::command_pool, command_buffers.size(), command_buffers.data());
}

void CommandBuffer::begin(VkCommandBufferUsageFlagBits usage, size_t index)
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;

	Graphics::vulkanAssert(vkBeginCommandBuffer(command_buffers[index], &begin_info));

	graphics_state.command_buffer = &command_buffers[index];
}

void CommandBuffer::end(size_t index)
{
	Graphics::vulkanAssert(vkEndCommandBuffer(command_buffers[index]));

	recorded[index] = true;
	graphics_state.command_buffer = nullptr;
}

void CommandBuffer::submit(size_t index, const std::vector<VkSemaphore>& wait_semaphores, const std::vector<VkSemaphore>& signal_semaphores, VkPipelineStageFlags* wait_stages, VkFence* fence)
{
	if (graphics_state.command_buffer == &command_buffers[index])
		end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submit_info.waitSemaphoreCount = wait_semaphores.size();
	submit_info.pWaitSemaphores = wait_semaphores.data();

	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffers[index];

	submit_info.signalSemaphoreCount = signal_semaphores.size();
	submit_info.pSignalSemaphores = signal_semaphores.data();

	if (fence != VK_NULL_HANDLE)
	{
		Graphics::vulkanAssert(vkResetFences(*Graphics::logical_device, 1, fence));
	}

	Graphics::vulkanAssert(vkQueueSubmit(Graphics::logical_device->getGraphicsQueue(), 1, &submit_info, fence ? *fence : VK_NULL_HANDLE));
}

void CommandBuffer::wait(VkQueue queue)
{
	if (queue != VK_NULL_HANDLE)
		vkQueueWaitIdle(queue);
	else
		vkQueueWaitIdle(Graphics::logical_device->getGraphicsQueue());
}
