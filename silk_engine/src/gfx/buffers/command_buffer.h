#pragma once

#include "gfx/allocators/command_pool.h"

struct CommandBufferSubmitInfo
{
	vk::Fence fence = VK_NULL_HANDLE;
	std::vector<vk::Semaphore> wait_semaphores = {};
	std::vector<vk::Semaphore> signal_semaphores = {};
	vk::PipelineStageFlags* wait_stages = nullptr;
};

class CommandBuffer : NonCopyable
{
public:
	CommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary, vk::QueueFlagBits queue_type = vk::QueueFlagBits::eGraphics);
	~CommandBuffer();

	void begin(vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	void end();

	void submit(const CommandBufferSubmitInfo& info = {});
	void submitIdle();

	bool wasRecorded() const { return recorded; }
	operator const vk::CommandBuffer& () const { return command_buffer; }

private:
	vk::Queue getQueue() const;

private:
	vk::CommandBuffer command_buffer;
	vk::CommandBufferLevel level;
	vk::QueueFlagBits queue_type;
	shared<CommandPool> pool;
	bool recorded = false;
};