#include "command_queue.h"

void CommandQueue::submit(std::function<void(CommandBuffer&)>&& command)
{
	//if (cb.getState() == CommandBuffer::State::PENDING || cb.getState() == CommandBuffer::State::INVALID)
	//	thread_data.pool.reset();
	thread_data.buffer.begin();
	std::forward<std::function<void(CommandBuffer&)>>(command)(thread_data.buffer);
}

void CommandQueue::execute()
{
	thread_data.buffer.submitImmidiatly();
	thread_data.buffer.begin();
}

void CommandQueue::execute(const CommandBuffer::SubmitInfo& submit_info)
{
	thread_data.buffer.submit(submit_info);
}
