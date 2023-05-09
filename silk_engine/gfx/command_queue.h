#pragma once

#include "gfx/buffers/command_buffer.h"

class CommandPool;

template <typename Func>
concept Command = requires (Func && val, CommandBuffer & cb) { { val(cb) } -> std::same_as<void>; };

class CommandQueue : NoCopy
{
public:
	CommandQueue(std::optional<uint32_t> queue_family_index, VkQueueFlagBits queue_type);

	template <Command Func>
	void record(Func&& command)
	{
		auto& command_buffer = getCommandBuffer();
		command_buffer.begin();
		command(command_buffer);
	}
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