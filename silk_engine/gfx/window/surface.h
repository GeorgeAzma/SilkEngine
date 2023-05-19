#pragma once

class Window;

class Surface : NoCopy
{
public:
	Surface(const Window& window);
	~Surface();

	void updateCapabilities();

	VkSurfaceCapabilitiesKHR getCapabilities() const { return capabilities; }
	VkSurfaceFormatKHR getSurfaceFormat() const { return format; }
	std::vector<VkSurfaceFormatKHR> getFormats() const { return formats; }
	std::vector<VkPresentModeKHR> getPresentModes() const { return present_modes; }
	VkFormat getFormat() const { return format.format; }
	VkColorSpaceKHR getColorSpace() const { return format.colorSpace; }
	int getPresentQueue() const { return present_queue; }
	bool isSupported() const { return formats.size(); }

	operator const VkSurfaceKHR& () const { return surface; }

private:
	VkSurfaceKHR surface = nullptr;
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats{};
	std::vector<VkPresentModeKHR> present_modes{};
	int present_queue = -1;
	VkSurfaceFormatKHR format{};
};