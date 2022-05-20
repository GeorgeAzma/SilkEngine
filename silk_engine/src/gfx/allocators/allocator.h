#pragma once

class Allocator : NonCopyable
{
public:
	Allocator();
	~Allocator();

	static bool needsStaging(VmaMemoryUsage usage);

	operator const VmaAllocator& () const { return allocator; }

private:
	VmaAllocator allocator = VK_NULL_HANDLE;
};