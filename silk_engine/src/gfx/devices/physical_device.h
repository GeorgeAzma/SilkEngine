#pragma once

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> transfer;
	std::optional<uint32_t> present;
	std::optional<uint32_t> compute;

	bool isSuitable() const;

	std::vector<uint32_t> getIndices() const;
};

class PhysicalDevice : NonCopyable
{
public:
	PhysicalDevice();

	VkDevice createLogicalDevice(const VkDeviceCreateInfo& create_info) const;

	void updateSurfaceDetails();
	void updateSurfaceCapabilities();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

	const QueueFamilyIndices& getQueueFamilyIndices() const { return queue_family_indices; }
	const VkPhysicalDeviceProperties& getProperties() const { return properties; }
	const VkPhysicalDeviceFeatures& getFeatures() const { return features; }
	VkSampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	VkSurfaceCapabilitiesKHR getSurfaceCapabilities() const { return surface_capabilities; }
	std::vector<VkSurfaceFormatKHR> getSurfaceFormats() const { return surface_formats; }
	std::vector<VkPresentModeKHR> getPresentModes() const { return present_modes; }
	VkFormatProperties getFormatProperties(VkFormat format) const;
	VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tilling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
	VkFormat getDepthFormat() const { return depth_format; }
	VkFormat getStencilFormat() const { return stencil_format; }
	operator const VkPhysicalDevice& () const { return physical_device; }

private:
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device);
	static VkPhysicalDevice chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	static int ratePhysicalDevice(VkPhysicalDevice physical_device);
	static bool checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device);
	static VkSampleCountFlagBits getMaxSampleCount(VkSampleCountFlags counts);	
	VkFormat findDepthFormat() const;
	VkFormat findStencilFormat() const;

private:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;

	QueueFamilyIndices queue_family_indices = {};

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkSampleCountFlagBits max_usable_sample_count;

	VkSurfaceCapabilitiesKHR surface_capabilities;
	std::vector<VkSurfaceFormatKHR> surface_formats;
	std::vector<VkPresentModeKHR> present_modes;

	VkFormat depth_format;
	VkFormat stencil_format;
};