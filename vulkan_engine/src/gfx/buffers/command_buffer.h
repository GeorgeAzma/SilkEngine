#pragma once

class CommandBuffer : NonCopyable
{
public:
	CommandBuffer(size_t count = 1);
	~CommandBuffer();

	void begin(VkCommandBufferUsageFlagBits usage = {}, size_t index = 0);
	void end(size_t index = 0);

	void submit(size_t index = 0, const std::vector<VkSemaphore>& wait_semaphores = {}, const std::vector<VkSemaphore>& signal_semaphores = {}, VkPipelineStageFlags* wait_stages = nullptr, VkFence* fence = nullptr);
	void wait(VkQueue queue = VK_NULL_HANDLE);

	const std::vector<VkCommandBuffer>& getCommandBuffers() const { return command_buffers; }
	operator const VkCommandBuffer& () const { return command_buffers[0]; }

private:
	std::vector<VkCommandBuffer> command_buffers;
};