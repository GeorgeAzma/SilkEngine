#pragma once

#include "gfx/allocators/allocation.h"

class Buffer : NonCopyable
{
public:
	typedef VkBufferUsageFlags Usage;
	enum UsageBits : Usage
	{
		TRANSFER_SRC = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		TRANSFER_DST = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		UNIFORM_TEXEL = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
		STORAGE_TEXEL = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
		UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		INDIRECT = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		SHADER_DEVICE_ADDRESS = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
	};

public:
	Buffer(VkDeviceSize size, Usage usage, const Allocation::Props& allocation_props = {});
	virtual ~Buffer();

	void resize(VkDeviceSize size);
	void reallocate(VkDeviceSize size);
	bool copy(VkBuffer destination, VkDeviceSize size = 0, VkDeviceSize offset = 0, VkDeviceSize dst_offset = 0) const;

	void bindVertex(uint32_t first = 0, VkDeviceSize offset = 0);
	void bindIndex(VkIndexType index_type = VK_INDEX_TYPE_UINT32, VkDeviceSize offset = 0);
	void drawIndirect(uint32_t index);
	void drawIndexedIndirect(uint32_t index);

	bool setData(const void* data, VkDeviceSize size = 0, VkDeviceSize offset = 0);
	void getData(void* data, VkDeviceSize size = 0) const;

	VkDeviceSize getSize() const { return ci.size; }
	Allocation getAllocation() const { return allocation; }
	VkDescriptorBufferInfo getDescriptorInfo() const { return { buffer, 0, ci.size }; }
	operator const VkBuffer& () const { return buffer; }

public:
	static void copy(VkBuffer destination, VkBuffer source, VkDeviceSize size, VkDeviceSize dst_offset = 0, VkDeviceSize src_offset = 0);

protected:
	static void insertMemoryBarrier(const VkBuffer& buffer, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkDeviceSize offset, VkDeviceSize size);

protected:
	VkBuffer buffer = nullptr;
	VkBufferCreateInfo ci{};
	VmaAllocationCreateInfo alloc_ci{};
	Allocation allocation{};
};