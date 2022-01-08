#include "buffer_utils.h"
#include "gfx/graphics.h"

void BufferUtils::copy(VkBuffer destination, VkBuffer source, size_t size)
{
	VkCommandBufferAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocation_info.commandPool = *Graphics::command_pool; //Creating seperate command pool with VK_COMMAND_POOL_CREATE_TRANSIENT_BIT might be more efficient
	allocation_info.commandBufferCount = 1;

	CommandBuffer command_buffer;

	command_buffer.begin(VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copy_region{};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, source, destination, 1, &copy_region);

	command_buffer.end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &(const VkCommandBuffer&)command_buffer;

	vkQueueSubmit(Graphics::logical_device->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(Graphics::logical_device->getGraphicsQueue());
}
