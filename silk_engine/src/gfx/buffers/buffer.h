#pragma once

#include "gfx/graphics.h"
#include "gfx/allocators/allocator.h"

class Buffer : NonCopyable
{
public:
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vma_usage);
	virtual ~Buffer();

	void setData(const void* data, size_t size = 0, size_t offset = 0);
	void setDataChecked(const void* data, size_t size = 0, size_t offset = 0);

	template<typename T>
	void getData(std::vector<T>& data, size_t size = 0) const
	{
		SK_ASSERT(size / sizeof(T),
			"Vulkan: Invalid data type size, buffer size isn't divisible by it");
		data.resize(size / sizeof(T));
		void* buffer_data;
		vmaMapMemory(*Graphics::allocator, allocation, &buffer_data);
		std::memcpy(data.data(), buffer_data, size ? size : this->size);
		vmaUnmapMemory(*Graphics::allocator, allocation);
	}

	size_t getSize() const { return size; }

	operator const VkBuffer& () const { return buffer; }

public:
	static void copy(VkBuffer destination, VkBuffer source, size_t size);
	static uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

public:
	const VkDeviceSize size;

protected:
	VkBuffer buffer;
	VmaAllocation allocation;

private:
	uint8_t* data = nullptr;
};