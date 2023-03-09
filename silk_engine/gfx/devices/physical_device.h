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
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

	const QueueFamilyIndices& getQueueFamilyIndices() const { return queue_family_indices; }
	const VkPhysicalDeviceProperties& getProperties() const { return properties; }
	const VkPhysicalDeviceFeatures& getFeatures() const { return features; }
	VkSampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	VkFormatProperties getFormatProperties(VkFormat format) const;
	VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tilling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
	operator const VkPhysicalDevice& () const { return physical_device; }

private:
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical_device);
	static VkPhysicalDevice chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	static int ratePhysicalDevice(VkPhysicalDevice physical_device);
	static bool checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device);
	static VkSampleCountFlagBits getMaxSampleCount(VkSampleCountFlags counts);

private:
	VkPhysicalDevice physical_device = nullptr;

	QueueFamilyIndices queue_family_indices = {};

	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkSampleCountFlagBits max_usable_sample_count;
};