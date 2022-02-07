#pragma once

class CommandPool : NonCopyable
{
public:
	CommandPool(VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	~CommandPool();

	void bind() const;

	operator const VkCommandPool& () const { return command_pool; }

private:
	VkCommandPool command_pool;
};