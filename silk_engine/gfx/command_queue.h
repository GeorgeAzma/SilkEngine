#pragma once

#include "silk_engine/gfx/buffers/command_buffer.h"

class CommandPool;

class CommandQueue : NoCopy
{
public:
	CommandQueue(const std::optional<uint32_t>& queue_family_index, VkQueueFlagBits queue_type);

	CommandBuffer& getCommandBuffer(bool begin = true);
	CommandBuffer& getNewCommandBuffer(bool begin = true);

	const shared<CommandBuffer>& submit(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	void execute(const std::vector<PipelineStage>& wait_stages = {}, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {});
	void reset();

private:
	VkQueueFlagBits queue_type;
	shared<CommandPool> command_pool = nullptr;
	std::vector<shared<CommandBuffer>> command_buffers;
	size_t index = 0;
};