#include "command_buffer.h"
#include "gfx/graphics.h"
#include "gfx/graphics_state.h"

CommandBuffer::CommandBuffer(size_t size)
{
	command_buffers.resize(size);

	VkCommandBufferAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation_info.commandPool = *Graphics::command_pool; //Creating seperate command pool with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT might be more efficient
	allocation_info.commandBufferCount = command_buffers.size();

	Graphics::vulkanAssert(vkAllocateCommandBuffers(*Graphics::logical_device, &allocation_info, command_buffers.data()));
}

CommandBuffer::~CommandBuffer()
{
	if (graphics_state.command_buffer) 
		end();

	vkFreeCommandBuffers(*Graphics::logical_device, *Graphics::command_pool, command_buffers.size(), command_buffers.data());
}

void CommandBuffer::begin(VkCommandBufferUsageFlagBits usage, size_t index)
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = usage;

	Graphics::vulkanAssert(vkBeginCommandBuffer(command_buffers[index], &begin_info));

	graphics_state.command_buffer = &command_buffers[index];
}

void CommandBuffer::end(size_t index)
{
	Graphics::vulkanAssert(vkEndCommandBuffer(command_buffers[index]));

	graphics_state.command_buffer = nullptr;
}