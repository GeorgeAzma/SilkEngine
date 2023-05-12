#include "allocator.h"
#include "allocation.h"
#include "gfx/render_context.h"
#include "gfx/instance.h"
#include "gfx/devices/logical_device.h"
#include "gfx/devices/physical_device.h"

Allocator::Allocator(const LogicalDevice& logical_device)
{
	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.vulkanApiVersion = uint32_t(RenderContext::getInstance().getVulkanVersion());
	allocator_info.instance = RenderContext::getInstance();
	allocator_info.physicalDevice = logical_device.getPhysicalDevice();
	allocator_info.device = logical_device;
	allocator_info.flags = logical_device.hasExtension(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) * VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
	vmaCreateAllocator(&allocator_info, &allocator);
}

Allocator::~Allocator()
{
	vmaDestroyAllocator(allocator);
}

void Allocator::map(VmaAllocation allocation, void** data) const
{
	RenderContext::vulkanAssert(vmaMapMemory(allocator, allocation, data));
}

void Allocator::unmap(VmaAllocation allocation) const
{
	vmaUnmapMemory(allocator, allocation);
}

VkMemoryPropertyFlags Allocator::getAllocationProperties(VmaAllocation allocation) const
{
	VkMemoryPropertyFlags mem_prop_flags = 0;
	vmaGetAllocationMemoryProperties(allocator, allocation, &mem_prop_flags);
	return mem_prop_flags;
}

Allocation Allocator::allocateBuffer(const VkBufferCreateInfo& buffer_create_info, const VmaAllocationCreateInfo& alloc_ci, VkBuffer& buffer) const
{
	VmaAllocation alloc = nullptr;
	RenderContext::vulkanAssert(vmaCreateBuffer(allocator, &buffer_create_info, &alloc_ci, &buffer, &alloc, nullptr));
	return Allocation(alloc);
}

Allocation Allocator::allocateImage(const VkImageCreateInfo& image_create_info, const VmaAllocationCreateInfo& alloc_ci, VkImage& image) const
{
	VmaAllocation alloc = nullptr;
	RenderContext::vulkanAssert(vmaCreateImage(allocator, &image_create_info, &alloc_ci, &image, &alloc, nullptr));
	return Allocation(alloc);
}

void Allocator::destroyBuffer(VkBuffer buffer, VmaAllocation allocation) const
{
	vmaDestroyBuffer(allocator, buffer, allocation);
}

void Allocator::destroyImage(VkImage image, VmaAllocation allocation) const
{
	vmaDestroyImage(allocator, image, allocation);
}
