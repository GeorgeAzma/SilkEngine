#pragma once

#include "silk_engine/gfx/allocators/allocation.h"

class Buffer : NoCopy
{
public:
	struct Range
	{
		void* data = nullptr; 
		VkDeviceSize size = 0;
		VkDeviceSize offset = 0;
	};

public:
	Buffer(VkDeviceSize size, BufferUsage usage, const Allocation::Props& allocation_props = {});
	virtual ~Buffer();

	void resize(VkDeviceSize size);
	void reallocate(VkDeviceSize size);
	void copy(VkBuffer destination, VkDeviceSize size = 0, VkDeviceSize offset = 0, VkDeviceSize dst_offset = 0) const;

	void bindVertex(uint32_t first = 0, VkDeviceSize offset = 0);
	void bindIndex(VkIndexType index_type = VK_INDEX_TYPE_UINT32, VkDeviceSize offset = 0);
	void drawIndirect(uint32_t index);
	void drawIndexedIndirect(uint32_t index);

	VkDeviceSize getSize() const { return ci.size; }
	const Allocation& getAllocation() const { return allocation; }
	VkDescriptorBufferInfo getDescriptorInfo() const { return { buffer, 0, ci.size }; }
	operator const VkBuffer& () const { return buffer; }

	bool setData(const void* data, VkDeviceSize size = 0, VkDeviceSize offset = 0);
	void getData(void* data, VkDeviceSize size = 0, VkDeviceSize offset = 0) const;
	void getDataRanges(const std::vector<Range>& ranges) const;

	void barrier(VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, PipelineStage source_stage, PipelineStage destination_stage, VkDeviceSize offset = 0, VkDeviceSize size = 0) const;

public:
	static void copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset = 0, VkDeviceSize src_offset = 0);

protected:
	static void barrier(const VkBuffer& buffer, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, PipelineStage source_stage, PipelineStage destination_stage, VkDeviceSize offset, VkDeviceSize size);

protected:
	VkBuffer buffer = nullptr;
	VkBufferCreateInfo ci;
	VmaAllocationCreateInfo alloc_ci;
	Allocation allocation;
};