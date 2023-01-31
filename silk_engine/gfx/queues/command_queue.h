#pragma once

#include "gfx/allocators/command_pool.h"
#include "gfx/buffers/command_buffer.h"

class CommandQueue : NonCopyable
{
	struct ThreadData
	{
		ThreadData()
			: pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT), buffer(pool)
		{

		}
		CommandPool pool;
		CommandBuffer buffer;
	};

public:
	void submit(std::function<void(CommandBuffer&)>&& command);
	void execute();
	void execute(const CommandBuffer::SubmitInfo& submit_info);

private:
	ThreadData thread_data{};
};