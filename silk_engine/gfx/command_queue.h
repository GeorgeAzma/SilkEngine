#pragma once

#include "gfx/buffers/command_buffer.h"

class CommandPool;

class CommandQueue : NonCopyable
{
public:
	CommandQueue(std::optional<uint32_t> queue_family_index, VkQueueFlagBits queue_type);

	void submit(std::function<void(CommandBuffer&)>&& command);
	void execute();
	void execute(const CommandBuffer::SubmitInfo& submit_info);
	void reset();

private:
	CommandBuffer& getCommandBuffer();

private:
	VkQueueFlagBits queue_type;
	shared<CommandPool> command_pool;
	std::vector<shared<CommandBuffer>> command_buffers;
	size_t index;
};