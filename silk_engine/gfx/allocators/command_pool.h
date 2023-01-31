#pragma once

class CommandPool : NonCopyable
{
public:
	CommandPool(VkCommandPoolCreateFlags flags = {}, std::optional<uint32_t> queue_family_index = {});
	~CommandPool();

	VkCommandBuffer allocate(VkCommandBufferLevel level);
	void deallocate(const VkCommandBuffer& command_buffer);
	void reset();

	operator const VkCommandPool& () const { return command_pool; }
	uint32_t allocatedCommandBufferCount() const { return allocated_command_buffer_count; }

private:
	VkCommandPool command_pool = nullptr;
	uint32_t allocated_command_buffer_count = 0;
};