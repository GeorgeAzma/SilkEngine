#pragma once

#include <vk_mem_alloc.h>

class Buffer : NonCopyable
{
public:
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
	virtual ~Buffer();

	void setData(const void* data, size_t size = 0);
	size_t getSize() const { return size; }

	operator const VkBuffer& () const { return buffer; }

public:
	static void copy(VkBuffer destination, VkBuffer source, size_t size);
	static void setData(const void* data, size_t size, VmaAllocation allocation);
	static uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

public:
	const VkDeviceSize size;

protected:
	VkBuffer buffer;
	VmaAllocation allocation;
};