#pragma once

class BufferUtils
{
public:
	static void copy(VkBuffer destination, VkBuffer source, size_t size);
};