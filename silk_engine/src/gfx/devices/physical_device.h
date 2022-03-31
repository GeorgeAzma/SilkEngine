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

	vk::Device createLogicalDevice(const vk::DeviceCreateInfo& create_info) const;

	void updateSurfaceDetails();
	void updateSurfaceCapabilities();
	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	const QueueFamilyIndices& getQueueFamilyIndices() const { return queue_family_indices; }
	const vk::PhysicalDeviceProperties& getProperties() const { return properties; }
	const vk::PhysicalDeviceFeatures& getFeatures() const { return features; }
	vk::SampleCountFlagBits getMaxSampleCount() const { return max_usable_sample_count; }
	vk::SurfaceCapabilitiesKHR getSurfaceCapabilities() const { return surface_capabilities; }
	std::vector<vk::SurfaceFormatKHR> getSurfaceFormats() const { return surface_formats; }
	std::vector<vk::PresentModeKHR> getPresentModes() const { return present_modes; }
	vk::FormatProperties getFormatProperties(vk::Format format) const;
	vk::ImageFormatProperties getImageFormatProperties(vk::Format format, vk::ImageType type, vk::ImageTiling tilling, vk::ImageUsageFlags usage, vk::ImageCreateFlags flags) const;
	vk::Format getDepthFormat() const { return depth_format; }
	vk::Format getStencilFormat() const { return stencil_format; }
	operator const vk::PhysicalDevice& () const { return physical_device; }

private:
	static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physical_device);
	static vk::PhysicalDevice chooseMostSuitablePhysicalDevice(const std::vector<vk::PhysicalDevice>& physical_devices);
	static int ratePhysicalDevice(vk::PhysicalDevice physical_device);
	static bool checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, vk::PhysicalDevice physical_device);
	static vk::SampleCountFlagBits getMaxSampleCount(vk::SampleCountFlags counts);	
	vk::Format findDepthFormat() const;
	vk::Format findStencilFormat() const;

private:
	vk::PhysicalDevice physical_device = VK_NULL_HANDLE;

	QueueFamilyIndices queue_family_indices = {};

	vk::PhysicalDeviceProperties properties;
	vk::PhysicalDeviceFeatures features;
	vk::SampleCountFlagBits max_usable_sample_count;

	vk::SurfaceCapabilitiesKHR surface_capabilities;
	std::vector<vk::SurfaceFormatKHR> surface_formats;
	std::vector<vk::PresentModeKHR> present_modes;

	vk::Format depth_format;
	vk::Format stencil_format;
};