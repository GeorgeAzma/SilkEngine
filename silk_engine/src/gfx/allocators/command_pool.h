#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>

class CommandPool : NonCopyable
{
public:
	CommandPool(vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, std::optional<uint32_t> queue_family_index = {});
	~CommandPool();

	vk::CommandBuffer allocate(vk::CommandBufferLevel level);
	void deallocate(const vk::CommandBuffer& command_buffer);

	operator const vk::CommandPool& () const { return command_pool; }
	uint32_t allocatedCommandBufferCount() const { return allocated_command_buffer_count; }

private:
	vk::CommandPool command_pool = VK_NULL_HANDLE;
	uint32_t allocated_command_buffer_count = 0;
};