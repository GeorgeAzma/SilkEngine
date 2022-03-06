#pragma once

class Allocator
{
public:
	Allocator();
	~Allocator();

	operator const VmaAllocator& () const { return allocator; }
private:
	VmaAllocator allocator;
};