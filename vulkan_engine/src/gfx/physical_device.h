#pragma once

#include <optional>
#include <vector>
#include <type_traits>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;

	bool isSuitable() const
	{
		return graphics.has_value() && present.has_value();
	}

	std::vector<uint32_t> getIndices() const 
	{ 
		if (!isSuitable()) 
			return {};

		return { *graphics, *present };
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

	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	VkFormat findStencilFormat();

private:
	std::vector<VkPhysicalDevice> getAvailablePhysicalDevices() const;
	void chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	int ratePhysicalDevice(VkPhysicalDevice physical_device);
	std::vector<VkExtensionProperties> getAvailablePhysicalDeviceExtensions(VkPhysicalDevice physical_device) const;
	bool checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device) const;

private:
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;

	QueueFamilyIndices queue_family_indices = {};

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
};