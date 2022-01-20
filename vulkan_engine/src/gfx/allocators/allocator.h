#pragma once

#include <vk_mem_alloc.h>

class Allocator
{
public:
	Allocator();
	~Allocator();

	operator const VmaAllocator& () const { return allocator; }
private:
	VmaAllocator allocator;
};