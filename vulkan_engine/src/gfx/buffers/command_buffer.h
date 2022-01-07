#pragma once

class CommandBuffer
{
public:
	CommandBuffer(size_t size = 1);
	~CommandBuffer();

	void begin(VkCommandBufferUsageFlagBits usage = {}, size_t index = 0);
	void end(size_t index = 0);

	const std::vector<VkCommandBuffer>& getCommandBuffers() const { return command_buffers; }

	operator const VkCommandBuffer& () const { return command_buffers[0]; }

private:
	std::vector<VkCommandBuffer> command_buffers;
};