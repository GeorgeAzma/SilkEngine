#pragma once

class Surface
{
public:
	Surface(const VkInstance* instance, GLFWwindow* window);
	~Surface();

	const VkSurfaceKHR& getSurface() const { return surface; }

private:
	const VkInstance* instance = nullptr;
	VkSurfaceKHR surface;
};