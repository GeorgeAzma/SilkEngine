#pragma once

class Window;

class Surface : NonCopyable
{
public:
	Surface(const Window& window);
	~Surface();

	void updateCapabilities();

	VkSurfaceCapabilitiesKHR getCapabilities() const { return capabilities; }
	std::vector<VkSurfaceFormatKHR> getFormats() const { return formats; }
	std::vector<VkPresentModeKHR> getPresentModes() const { return present_modes; }
	VkSurfaceFormatKHR getFormat() const { return format; }
	int32_t getPresentQueue() const { return present_queue; }
	bool isSupported() const { return formats.size(); }

	operator const VkSurfaceKHR& () const { return surface; }

private:
	VkSurfaceKHR surface = nullptr;
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats{};
	std::vector<VkPresentModeKHR> present_modes{};
	int32_t present_queue = -1;
	VkSurfaceFormatKHR format{};
};