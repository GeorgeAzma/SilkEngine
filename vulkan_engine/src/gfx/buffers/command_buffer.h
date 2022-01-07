#pragma once

class CommandBuffer
{
public:
	CommandBuffer(size_t size);
	~CommandBuffer();

	void begin(VkCommandBufferUsageFlagBits usage = {}, size_t index = 0);
	void end(size_t index = 0);

	const std::vector<VkCommandBuffer>& getCommandBuffers() const { return command_buffers; }

private:
	std::vector<VkCommandBuffer> command_buffers;
};