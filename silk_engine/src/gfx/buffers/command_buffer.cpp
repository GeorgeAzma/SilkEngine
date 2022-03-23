#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/allocators/command_pool.h"

CommandBuffer::CommandBuffer(vk::CommandBufferLevel level, vk::QueueFlagBits queue_type)
	: level(level), queue_type(queue_type), pool(Graphics::getCommandPool()), 
	vk::CommandBuffer(VkCommandBuffer(Graphics::logical_device->allocateCommandBuffers({ *Graphics::getCommandPool(), level, 1 }).front())),
	is_primary(level == vk::CommandBufferLevel::ePrimary)
{
}

CommandBuffer::~CommandBuffer()
{
	Graphics::logical_device->freeCommandBuffers(*pool, { *this });
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

	vk::CommandBuffer::begin(vk::CommandBufferBeginInfo(usage, &inheritance_info));
	Graphics::setActiveCommandBuffer(this);
	if(is_primary)
		Graphics::setActivePrimaryCommandBuffer(this);
	running = true;
}

void CommandBuffer::end()
{
	if (!running)
		return;

	vk::CommandBuffer::end();
	Graphics::setActiveCommandBuffer(nullptr);
	if (is_primary)
		Graphics::setActivePrimaryCommandBuffer(nullptr);
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
	submit_info.pCommandBuffers = this;
	
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
	submit_info.pCommandBuffers = this;

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
