#pragma once

#include "gfx/allocators/allocator.h"

class Buffer : NonCopyable
{
public:
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
	virtual ~Buffer();

	void resize(VkDeviceSize size);

	void map(void** data) const;
	void unmap() const;

	void setData(const void* data, size_t size = 0, size_t offset = 0);
	void setDataChecked(const void* data, size_t size = 0, size_t offset = 0);

	void getData(void* data, size_t size = 0) const;
	bool isMapped() const { return mapped; }
	VkDeviceSize getSize() const { return size; }

	VmaAllocation getAllocation() const { return allocation; }
	operator VkDescriptorBufferInfo () const { return { buffer, 0, size }; }
	operator const VkBuffer& () const { return buffer; }

public:
	static void copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset = 0, VkDeviceSize src_offset = 0);

protected:
	static void insertMemoryBarrier(const VkBuffer& buffer, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDeviceSize offset, VkDeviceSize size);

protected:
	VkBuffer buffer;
	VkDeviceSize size;

private:
	VkBufferCreateInfo create_info{};
	VmaAllocationCreateInfo allocation_create_info{};
	VmaAllocation allocation;
	uint8_t* data = nullptr;
	const bool needs_staging = false;
	mutable bool mapped = false;
};