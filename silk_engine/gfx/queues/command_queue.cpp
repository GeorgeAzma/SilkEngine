#include "command_queue.h"

void CommandQueue::submit(std::function<void(CommandBuffer&)>&& command)
{
	CommandBuffer& cb = thread_data.buffer;
	//if (cb.getState() == CommandBuffer::State::PENDING || cb.getState() == CommandBuffer::State::INVALID)
	//	thread_data.pool.reset();
	cb.begin(0);
	std::forward<std::function<void(CommandBuffer&)>>(command)(cb);
}

void CommandQueue::execute()
{
	thread_data.buffer.submitImmidiatly();
	thread_data.buffer.begin(0);
}

void CommandQueue::execute(const CommandBuffer::SubmitInfo& submit_info)
{
	thread_data.buffer.submit(submit_info);
}
