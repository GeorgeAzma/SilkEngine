#pragma once

class Surface
{
public:
	Surface(GLFWwindow* window);
	~Surface();

	operator const VkSurfaceKHR& () const { return surface; }
	const VkSurfaceCapabilitiesKHR& getCapabilities() const { return capabilities; }
	const std::vector<VkSurfaceFormatKHR>& getFormats() const { return formats; }
	const std::vector<VkPresentModeKHR>& getPresentModes() const { return present_modes; }

	void getSupportDetails(VkPhysicalDevice physical_device);

private:
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};