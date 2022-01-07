#include "staging_buffer.h"
#include "command_buffer.h"
#include "gfx/graphics.h"
#include "utils/debug_timer.h"

StagingBuffer::StagingBuffer(const void* data, VkDeviceSize size)
    : Buffer(size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
    setData(data);
}

void StagingBuffer::copy(VkBuffer destination) const //~0.8ms
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
	vkCmdCopyBuffer(command_buffer, buffer, destination, 1, &copy_region);

	command_buffer.end();

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &(const VkCommandBuffer&)command_buffer;

	vkQueueSubmit(Graphics::logical_device->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(Graphics::logical_device->getGraphicsQueue());
}
