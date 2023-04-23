#pragma once

class CommandPool : NonCopyable
{
public:
	CommandPool(VkCommandPoolCreateFlags flags = {}, std::optional<uint32_t> queue_family_index = {});
	~CommandPool();

	VkCommandBuffer allocate(VkCommandBufferLevel level);
	void deallocate(const VkCommandBuffer& command_buffer);
	void reset(bool free = false);

	operator const VkCommandPool& () const { return command_pool; }
	uint32_t allocatedCommandBufferCount() const { return allocated_command_buffer_count; }
	VkCommandPoolCreateFlags getFlags() const { return flags; }

private:
	VkCommandPool command_pool = nullptr;
	VkCommandPoolCreateFlags flags = VkCommandPoolCreateFlags(0);
	uint32_t allocated_command_buffer_count = 0;
};