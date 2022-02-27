#pragma once

#include <vulkan/vulkan.hpp>

class CommandPool : NonCopyable
{
public:
	CommandPool(vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, std::optional<uint32_t> queue_family_index = {});
	~CommandPool();

	operator const vk::CommandPool& () const { return command_pool; }

private:
	vk::CommandPool command_pool;
};