#pragma once

class CommandPool : NonCopyable
{
public:
	CommandPool();
	~CommandPool();

	operator const VkCommandPool& () const { return command_pool; }

private:
	VkCommandPool command_pool;
};