#pragma once

class Allocator : NonCopyable
{
public:
	Allocator();
	~Allocator();

	operator const VmaAllocator& () const { return allocator; }

private:
	VmaAllocator allocator;
};