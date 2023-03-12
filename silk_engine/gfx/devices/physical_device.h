#pragma once

class PhysicalDevice : NonCopyable
{
	friend class LogicalDevice;
public:
	PhysicalDevice(VkPhysicalDevice physical_device);
	PhysicalDevice();

	bool supportsExtension(const char* extension_name) const;

	VkDevice createLogicalDevice(const VkDeviceCreateInfo& create_info) const;
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	std::vector<VkSurfaceFormatKHR> getSurfaceFormats(VkSurfaceKHR surface) const;
	std::vector<VkPresentModeKHR> getSurfacePresentModes(VkSurfaceKHR surface) const;
	VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;
	bool getSurfaceSupport(uint32_t queue_index, VkSurfaceKHR surface) const;

	const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties() const { return queue_family_properties; }
	const std::vector<uint32_t>& getQueueFamilyIndices() const { return queue_family_indices; }
	const VkPhysicalDeviceProperties& getProperties() const { return properties; }
	const VkPhysicalDeviceFeatures& getFeatures() const { return features; }
	VkSampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	VkFormatProperties getFormatProperties(VkFormat format) const;
	VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tilling, VkImageUsageFlags usage, VkImageCreateFlags flags) const;
	int getGraphicsQueue() const { return graphics_queue; }
	int getComputeQueue() const { return compute_queue; }
	int getTransferQueue() const { return transfer_queue; }
	int getPresentQueue() const { return present_queue; }
	operator const VkPhysicalDevice& () const { return physical_device; }

private:
	void init(VkPhysicalDevice physical_device);

private:
	static VkPhysicalDevice chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices);
	static int ratePhysicalDevice(VkPhysicalDevice physical_device);
	static VkSampleCountFlagBits getMaxSampleCount(VkSampleCountFlags counts);

private:
	VkPhysicalDevice physical_device = nullptr;
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	std::vector<uint32_t> queue_family_indices; 
	std::unordered_map<std::string, uint32_t> available_extensions; // extension name | extension spec version
	int graphics_queue = -1;
	int compute_queue = -1;
	int transfer_queue = -1;
	int present_queue = -1;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkSampleCountFlagBits max_usable_sample_count;
};