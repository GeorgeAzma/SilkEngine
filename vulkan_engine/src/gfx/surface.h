#pragma once

class Surface
{
public:
	Surface(VkInstance* instance, GLFWwindow* window);
	~Surface();

private:
	VkInstance* instance = nullptr;
	VkSurfaceKHR surface;
};