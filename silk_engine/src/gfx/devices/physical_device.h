#pragma once

#include <optional>
#include <vector>
#include <type_traits>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> transfer;
	std::optional<uint32_t> present;
	std::optional<uint32_t> compute;

	bool isSuitable() const
	{
		return graphics.has_value() 
			&& transfer.has_value()
			&& present.has_value() 
			&& compute.has_value();
	}

	std::vector<uint32_t> getIndices() const 
	{ 
		if (!isSuitable()) 
			return {};

		return { *graphics, *transfer, *present, *compute };
	};
};

class PhysicalDevice : NonCopyable
{
public:
	PhysicalDevice();
	~PhysicalDevice();

	operator const VkPhysicalDevice& () const { return physical_device; }
	const QueueFamilyIndices& getQueueFamilyIndices() const { return queue_family_indices; }
	const VkPhysicalDeviceProperties& getProperties() const { return properties; }
	const VkPhysicalDeviceFeatures& getFeatures() const { return features; }
	VkSampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	VkSurfaceCapabilitiesKHR getSurfaceCapabilities() const { return surface_capabilities; }
	std::vector<VkSurfaceFormatKHR> getSurfaceFormats() const { return surface_formats; }
	std::vector<VkPresentModeKHR> getPresentModes() const { return present_modes; }
	VkFormatProperties getFormatProperties(VkFormat format) const; 
	VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tilling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
	void updateSurfaceDetails();
	void updateSurfaceCapabilities();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	VkFormat findDepthFormat() const;
	VkFormat findStencilFormat() const;

private:
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device);
	void chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	int ratePhysicalDevice(VkPhysicalDevice physical_device);
	std::vector<VkExtensionProperties> getAvailablePhysicalDeviceExtensions(VkPhysicalDevice physical_device) const;
	bool checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device) const;
	VkSampleCountFlagBits getMaxUsableSampleCount() const;
    void updateSurfaceDetails(VkPhysicalDevice physical_device);

private:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;

	QueueFamilyIndices queue_family_indices = {};

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkSampleCountFlagBits max_usable_sample_count;

	VkSurfaceCapabilitiesKHR surface_capabilities;
	std::vector<VkSurfaceFormatKHR> surface_formats;
	std::vector<VkPresentModeKHR> present_modes;
};