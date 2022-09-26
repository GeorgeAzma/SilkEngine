#pragma once

class Surface : NonCopyable
{
public:
	Surface();
	~Surface();

	operator const VkSurfaceKHR& () const { return surface; }

private:
	VkSurfaceKHR surface;
};