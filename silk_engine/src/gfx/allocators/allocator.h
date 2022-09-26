#pragma once

#include <vk_mem_alloc.h>

class Allocator : NonCopyable
{
public:
	Allocator();
	~Allocator();

	operator const VmaAllocator& () const { return allocator; }

private:
	VmaAllocator allocator = nullptr;
};