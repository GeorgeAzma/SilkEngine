#pragma once

#include "gfx/allocators/allocator.h"

class Buffer : NonCopyable
{
public:
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
	virtual ~Buffer();

	void setData(const void* data, size_t size = 0, size_t offset = 0);
	void setDataChecked(const void* data, size_t size = 0, size_t offset = 0);

	void getData(void* data, size_t size = 0) const;
	size_t getSize() const { return size; }

	operator const VkDescriptorBufferInfo& () const { return { buffer, 0, size }; }
	operator const VkBuffer& () const { return buffer; }

public:
	static void copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset = 0, VkDeviceSize src_offset = 0);
	static uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

public:
	const VkDeviceSize size;

protected:
	VkBuffer buffer;
	VmaAllocation allocation;

private:
	uint8_t* data = nullptr;
	const bool needs_staging = false;
};