#pragma once

#include "gfx/allocators/command_pool.h"
#include "gfx/buffers/command_buffer.h"

class CommandQueue : NonCopyable
{
public:
	void submit(std::function<void(CommandBuffer&)>&& command);
	void execute();
	void execute(const CommandBuffer::SubmitInfo& submit_info);

private:
	CommandPool pool = CommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	CommandBuffer command_buffer = CommandBuffer(pool);
};