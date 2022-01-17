#pragma once

struct CommandBufferSubmitInfo
{
	size_t index = 0;
	VkFence fence = VK_NULL_HANDLE;
	std::vector<VkSemaphore> wait_semaphores = {};
	std::vector<VkSemaphore> signal_semaphores = {};
	VkPipelineStageFlags* wait_stages = nullptr;
	VkQueue queue = VK_NULL_HANDLE; //Graphics queue by default
};

class CommandBuffer : NonCopyable
{
public:
	CommandBuffer(size_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	~CommandBuffer();

	void begin(VkCommandBufferUsageFlagBits usage = {}, size_t index = 0);
	void end(size_t index = 0);

	void submit(const CommandBufferSubmitInfo& command_buffer_submit_info = {});
	void wait();

	bool wasRecorded(size_t index) const { return recorded[index]; }
	const std::vector<VkCommandBuffer>& getCommandBuffers() const { return command_buffers; }
	operator const VkCommandBuffer& () const { return command_buffers[0]; }

private:
	std::vector<VkCommandBuffer> command_buffers;
	std::vector<bool> recorded;
	VkCommandBufferLevel level;
	VkFence fence = VK_NULL_HANDLE; 
	VkQueue queue = VK_NULL_HANDLE;
};