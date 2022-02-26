#pragma once

class Surface : NonCopyable
{
public:
	Surface();
	~Surface();

	operator const vk::SurfaceKHR& () const { return surface; }

private:
	vk::SurfaceKHR surface;
};