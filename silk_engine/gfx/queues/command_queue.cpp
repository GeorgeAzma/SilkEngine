#include "command_queue.h"

void CommandQueue::submit(std::function<void(CommandBuffer&)>&& command)
{
	//if (cb.getState() == CommandBuffer::State::PENDING || cb.getState() == CommandBuffer::State::INVALID)
	//	thread_data.pool.reset();
	command_buffer.begin();
	std::forward<std::function<void(CommandBuffer&)>>(command)(command_buffer);
}

void CommandQueue::execute()
{
	command_buffer.submitImmidiatly();
}

void CommandQueue::execute(const CommandBuffer::SubmitInfo& submit_info)
{
	command_buffer.submit(submit_info);
}
