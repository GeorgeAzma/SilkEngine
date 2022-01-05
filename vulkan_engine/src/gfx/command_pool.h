#pragma once

class CommandPool
{
public:
	CommandPool();
	~CommandPool();

private:
	VkCommandPool command_pool;
};