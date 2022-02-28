#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/command_pool.h"

CommandBuffer::CommandBuffer(vk::CommandBufferLevel level, vk::QueueFlagBits queue_type)
	: level(level), queue_type(queue_type), pool(Graphics::getCommandPool())
{
	vk::CommandBufferAllocateInfo allocate_info{};
	allocate_info.level = level;
	allocate_info.commandPool = *pool;
	allocate_info.commandBufferCount = 1;
	command_buffer = Graphics::logical_device->allocateCommandBuffers(allocate_info).front();
	is_primary = (level == vk::CommandBufferLevel::ePrimary);
}

CommandBuffer::~CommandBuffer()
{
	Graphics::logical_device->freeCommandBuffers(*pool, { command_buffer });
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags usage)
{
	if (running)
		return;

	vk::CommandBufferInheritanceInfo inheritance_info{};
	if (usage & vk::CommandBufferUsageFlagBits::eRenderPassContinue)
	{
		inheritance_info.renderPass = Graphics::active.render_pass;
		inheritance_info.subpass = Graphics::active.subpass;
		inheritance_info.framebuffer = Graphics::active.framebuffer;
		//inheritance_info.occlusionQueryEnable = ; //TODO:
		//inheritance_info.pipelineStatistics = ;  //TODO:
	}

	command_buffer.begin(vk::CommandBufferBeginInfo(usage, &inheritance_info));
	Graphics::active.command_buffer = command_buffer;
	if(is_primary)
		Graphics::active.primary_command_buffer = command_buffer;
	running = true;
}

void CommandBuffer::end()
{
	if (!running)
		return;

	command_buffer.end();
	Graphics::active.command_buffer = VK_NULL_HANDLE;
	if (is_primary)
		Graphics::active.primary_command_buffer = command_buffer;
	recorded = true;
	running = false;
}

void CommandBuffer::submit(const CommandBufferSubmitInfo& info)
{
	end();
	if (!recorded)
		return;

	vk::SubmitInfo submit_info{};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	
	submit_info.pWaitDstStageMask = info.wait_stages;
	submit_info.waitSemaphoreCount = info.wait_semaphores.size();
	submit_info.pWaitSemaphores = info.wait_semaphores.data();

	submit_info.signalSemaphoreCount = info.signal_semaphores.size();
	submit_info.pSignalSemaphores = info.signal_semaphores.data();

	if ((const VkFence&)info.fence != VK_NULL_HANDLE)
		Graphics::logical_device->resetFences({ info.fence });
	
	getQueue().submit({ submit_info }, info.fence);
}

void CommandBuffer::submitIdle()
{
	end();
	if (!recorded)
		return;

	vk::SubmitInfo submit_info{};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vk::Fence fence = Graphics::logical_device->createFence({});
	Graphics::logical_device->resetFences({ fence });

	getQueue().submit({ submit_info }, fence);	
	Graphics::logical_device->waitForFences({ fence });
	
	Graphics::logical_device->destroyFence(fence);
}

vk::Queue CommandBuffer::getQueue() const
{
	switch (queue_type) 
	{
	case vk::QueueFlagBits::eGraphics: return Graphics::logical_device->getGraphicsQueue(); 
	case vk::QueueFlagBits::eTransfer: return Graphics::logical_device->getTransferQueue();
	case vk::QueueFlagBits::eCompute: return Graphics::logical_device->getComputeQueue();
	}

	return Graphics::logical_device->getGraphicsQueue(); //Or nullptr
}
