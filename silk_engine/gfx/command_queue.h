#pragma once

#include "gfx/buffers/command_buffer.h"

class CommandPool;

class CommandQueue : NonCopyable
{
public:
	CommandQueue(std::optional<uint32_t> queue_family_index, VkQueueFlagBits queue_type);

	void record(std::function<void(CommandBuffer&)>&& command);
	void submit(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	void execute();
	void reset();

private:
	CommandBuffer& getCommandBuffer();

private:
	VkQueueFlagBits queue_type;
	shared<CommandPool> command_pool;
	std::vector<shared<CommandBuffer>> command_buffers;
	size_t index;
};