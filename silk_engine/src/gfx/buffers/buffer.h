#pragma once

#include "gfx/allocators/allocator.h"
#include <vulkan/vulkan.hpp>

class Buffer : NonCopyable
{
public:
	Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage vma_usage);
	virtual ~Buffer();

	void resize(vk::DeviceSize size);

	void map(void** data) const;
	void unmap() const;

	void setData(const void* data, size_t size = 0, size_t offset = 0);
	void setDataChecked(const void* data, size_t size = 0, size_t offset = 0);

	void getData(void* data, size_t size = 0) const;
	bool isMapped() const { return mapped; }
	vk::DeviceSize getSize() const { return size; }

	VmaAllocation getAllocation() const { return allocation; }
	operator vk::DescriptorBufferInfo () const { return { buffer, 0, size }; }
	operator const vk::Buffer& () const { return buffer; }

public:
	static void copy(vk::Buffer destination, vk::Buffer source, vk::DeviceSize size, vk::DeviceSize dst_offset = 0, vk::DeviceSize src_offset = 0);

protected:
	static void insertMemoryBarrier(const vk::Buffer& buffer, vk::AccessFlags source_access_mask, vk::AccessFlags destination_access_mask, vk::PipelineStageFlags source_stage_mask, vk::PipelineStageFlags destination_stage_mask, vk::DeviceSize offset, vk::DeviceSize size);

protected:
	vk::Buffer buffer;
	vk::DeviceSize size;

private:
	vk::BufferCreateInfo ci{};
	VmaAllocationCreateInfo allocation_create_info{};
	VmaAllocation allocation;
	uint8_t* data = nullptr;
	const bool needs_staging = false;
	mutable bool mapped = false;
};