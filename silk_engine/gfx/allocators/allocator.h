#pragma once

#include <vk_mem_alloc.h>

class LogicalDevice;
class Allocation;

class Allocator : NoCopy
{
public:
	Allocator(const LogicalDevice& logical_device);
	~Allocator();

	void map(VmaAllocation allocation, void** data) const;
	void unmap(VmaAllocation allocation) const;
	VkMemoryPropertyFlags getAllocationProperties(VmaAllocation allocation) const;

	Allocation allocateBuffer(const VkBufferCreateInfo& buffer_create_info, const VmaAllocationCreateInfo& alloc_ci, VkBuffer& buffer) const;
	Allocation allocateImage(const VkImageCreateInfo& image_create_info, const VmaAllocationCreateInfo& alloc_ci, VkImage& image) const;
	void destroyBuffer(VkBuffer buffer, VmaAllocation allocation) const;
	void destroyImage(VkImage image, VmaAllocation allocation) const;

	operator const VmaAllocator& () const { return allocator; }

private:
	VmaAllocator allocator = nullptr;
};