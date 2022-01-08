#pragma once

class Buffer : NonCopyable
{
public:
	Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	virtual ~Buffer();

	void setData(const void* data);
	size_t getSize() const { return size; }

	operator const VkBuffer& () const { return buffer; }

private:
	static uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

public:
	VkDeviceMemory memory;
	const VkDeviceSize size;

protected:
	VkBuffer buffer;
};