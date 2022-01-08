#pragma once

class BufferUtils
{
public:
	static void copy(VkBuffer destination, VkBuffer source, size_t size);
	static void setData(const void* data, size_t size, VkDeviceMemory memory);
};