#pragma once

class Surface
{
public:
	Surface(GLFWwindow* window);
	~Surface();

	const VkSurfaceKHR& getSurface() const { return surface; }

private:
	VkSurfaceKHR surface;
};