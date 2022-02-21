#pragma once

#include "gfx/allocators/command_pool.h"

struct CommandBufferSubmitInfo
{
	VkFence fence = VK_NULL_HANDLE;
	std::vector<VkSemaphore> wait_semaphores = {};
	std::vector<VkSemaphore> signal_semaphores = {};
	VkPipelineStageFlags* wait_stages = nullptr;
};

class CommandBuffer : NonCopyable
{
public:
	CommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, VkQueueFlagBits queue_type = VK_QUEUE_GRAPHICS_BIT);
	~CommandBuffer();

	void begin(VkCommandBufferUsageFlagBits usage = {});
	void end();

	void submit(const CommandBufferSubmitInfo& info = {});
	void submitIdle();

	operator const VkCommandBuffer& () const { return command_buffer; }

private:
	VkQueue getQueue() const;

private:
	VkCommandBuffer command_buffer;
	VkCommandBufferLevel level;
	VkQueueFlagBits queue_type;
	shared<CommandPool> pool;
	bool recorded = false;
};