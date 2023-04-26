#include "command_queue.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"
#include "gfx/allocators/command_pool.h"

CommandQueue::CommandQueue(std::optional<uint32_t> queue_family_index, VkQueueFlagBits queue_type)
	: queue_type(queue_type), command_pool(makeShared<CommandPool>(VkCommandPoolCreateFlags{}, queue_family_index)), command_buffers({ makeShared<CommandBuffer>(*command_pool) }), index(0)
{
}

void CommandQueue::record(std::function<void(CommandBuffer&)>&& command)
{
	auto& command_buffer = getCommandBuffer();
	command_buffer.begin();
	std::forward<std::function<void(CommandBuffer&)>>(command)(command_buffer);
}

void CommandQueue::submit(const Fence* fence, const std::vector<VkPipelineStageFlags>& wait_stages, const std::vector<VkSemaphore>& wait_semaphores, const std::vector<VkSemaphore>& signal_semaphores)
{
	command_buffers[index]->submit(fence, wait_stages, wait_semaphores, signal_semaphores, queue_type);
}

void CommandQueue::execute()
{
	command_buffers[index]->execute(queue_type);
}

void CommandQueue::reset()
{
	bool needs_rest = false;
	for (const auto& command_buffer : command_buffers)
		if (command_buffer->getState() != CommandBuffer::State::INITIAL)
		{
			needs_rest = true;
			break;
		}
	if (needs_rest)
	{
		command_pool->reset();
		for (const auto& command_buffer : command_buffers)
			command_buffer->reset();
	}
	index = 0;
}

CommandBuffer& CommandQueue::getCommandBuffer()
{
	shared<CommandBuffer> command_buffer = command_buffers[index];
	if (command_buffer->getState() != CommandBuffer::State::INITIAL && command_buffer->getState() != CommandBuffer::State::RECORDING)
	{
		++index;
		if (index >= command_buffers.size())
			command_buffers.emplace_back(makeShared<CommandBuffer>(*command_pool));
		command_buffer = command_buffers[index];
	}
	return *command_buffer;
}
