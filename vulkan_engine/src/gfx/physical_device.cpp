#include "physical_device.h"
#include "logical_device.h"
#include "swap_chain.h"
#include "graphics.h"

PhysicalDevice::PhysicalDevice()
{
	auto physical_devices = getAvailablePhysicalDevices();
	// a.k.a best gpu you have
	chooseMostSuitablePhysicalDevice(physical_devices);
}

PhysicalDevice::~PhysicalDevice()
{
}

// Find supported queue families of this device
QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
	QueueFamilyIndices queue_family_indices = {};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

	for (size_t i = 0; i < queue_family_count; ++i)
	{
		const auto& queue_family = queue_families[i];
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			queue_family_indices.graphics = i;

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
		if (present_support)
			queue_family_indices.present = i;

		if (queue_family_indices.isSuitable())
			break;
	}

	return queue_family_indices;
}

VkFormat PhysicalDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
		{
			return format;
		}
	}

	VE_CORE_ERROR("Vulkan: Couldn't find supported format");
	return VkFormat(0);
}

VkFormat PhysicalDevice::findDepthFormat()
{
	return findSupportedFormat
	(
		{ VK_FORMAT_D32_SFLOAT, 
		VK_FORMAT_D32_SFLOAT_S8_UINT, 
		VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

VkFormat PhysicalDevice::findStencilFormat()
{
	return findSupportedFormat
	(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

std::vector<VkPhysicalDevice> PhysicalDevice::getAvailablePhysicalDevices() const
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(Graphics::instance->getInstance(), &device_count, nullptr);

	VE_CORE_ASSERT(device_count > 0, 
		"Vulkan: Couldn't find GPU with vulkan support");

	std::vector<VkPhysicalDevice> physical_devices(device_count);
	vkEnumeratePhysicalDevices(Graphics::instance->getInstance(), &device_count, physical_devices.data());

	return physical_devices;
}

void PhysicalDevice::chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices)
{
	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : physical_devices)
	{
		int score = ratePhysicalDevice(device);
		if(score >= 0)
			candidates.insert(std::make_pair(score, device));
	}

	VE_CORE_ASSERT(candidates.rbegin()->first >= 0, 
		"Vulkan: Couldn't find suitable vulkan GPU");

	physical_device = candidates.rbegin()->second;
	vkGetPhysicalDeviceProperties(physical_device, &properties); 
	vkGetPhysicalDeviceFeatures(physical_device, &features);
	queue_family_indices = findQueueFamilies(physical_device, *Graphics::surface);
	Graphics::surface->getSupportDetails(physical_device);
	max_usable_sample_count = getMaxUsableSampleCount();
}

// This function decides which one is 
// the "best" gpu from all the gpu's user has 
// (mostly 1, sometimes 2 if it also has an integreted gpu)
int PhysicalDevice::ratePhysicalDevice(VkPhysicalDevice physical_device)
{
	int score = 0;

	QueueFamilyIndices queue_family_indices = findQueueFamilies(physical_device, *Graphics::surface);
	if (!queue_family_indices.isSuitable()) 
		return -1;

	bool extensions_supported = checkPhysicalDeviceExtensionSupport(LogicalDevice::getRequiredLogicalDeviceExtensions(), physical_device);
	if (!extensions_supported) 
		return -1;

	Graphics::surface->getSupportDetails(physical_device);
	bool is_swap_chain_adequate = !Graphics::surface->getFormats().empty() && 
		!Graphics::surface->getPresentModes().empty();
	if (!is_swap_chain_adequate) 
		return -1;

	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceFeatures physical_device_features;
	vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

	if (!physical_device_features.samplerAnisotropy) 
		return -1;

	bool is_discrete = physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

	score += is_discrete * 800;
	score += physical_device_features.geometryShader * 250;
	score += physical_device_features.tessellationShader * 150;
	score += physical_device_features.multiViewport * 100;
	score += physical_device_features.wideLines * 50;
	score += physical_device_features.largePoints * 50;
	score += physical_device_features.occlusionQueryPrecise * 25;

	return score;
}

std::vector<VkExtensionProperties> PhysicalDevice::getAvailablePhysicalDeviceExtensions(VkPhysicalDevice physical_device) const
{
	uint32_t extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

	return available_extensions;
}

bool PhysicalDevice::checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device) const
{
	auto available_extensions = getAvailablePhysicalDeviceExtensions(physical_device);

	for (const auto& required_extension : required_extensions)
	{
		bool extension_found = false;
		for (const auto& available_extension : available_extensions)
		{
			if (strcmp(required_extension, available_extension.extensionName) == 0)
			{
				extension_found = true;
			}
		}
		if (!extension_found)
		{
			return false;
		}
	}
	return true;
}

VkSampleCountFlagBits PhysicalDevice::getMaxUsableSampleCount() const
{
	const VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
	
	if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}