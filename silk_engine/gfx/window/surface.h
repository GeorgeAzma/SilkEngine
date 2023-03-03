#pragma once

class Window;

class Surface : NonCopyable
{
public:
	Surface(const Window& window);
	~Surface();

	void update(uint32_t width, uint32_t height);
	void updateCapabilities();

	VkSurfaceCapabilitiesKHR getCapabilities() const { return capabilities; }
	std::vector<VkSurfaceFormatKHR> getFormats() const { return formats; }
	std::vector<VkPresentModeKHR> getPresentModes() const { return present_modes; }

	operator const VkSurfaceKHR& () const { return surface; }

private:
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};