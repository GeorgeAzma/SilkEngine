#pragma once

#include "gfx/buffers/command_buffer.h"

class CommandPool;

class CommandQueue : NoCopy
{
public:
	CommandQueue(std::optional<uint32_t> queue_family_index, VkQueueFlagBits queue_type);

	CommandBuffer& getCommandBuffer(bool begin = true);

	void submit(const Fence* fence = nullptr, const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	void execute(const std::vector<VkPipelineStageFlags>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	void reset();

private:
	VkQueueFlagBits queue_type;
	shared<CommandPool> command_pool;
	std::vector<shared<CommandBuffer>> command_buffers;
	size_t index;
};