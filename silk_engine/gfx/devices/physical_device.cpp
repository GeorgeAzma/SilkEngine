#include "physical_device.h"
#include "logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/render_context.h"
#include "gfx/window/surface.h"
#include "gfx/window/window.h"
#include "gfx/instance.h"

PhysicalDevice::PhysicalDevice(const Instance& instance, VkPhysicalDevice physical_device)
	: instance(instance), physical_device(physical_device)
{
	vkGetPhysicalDeviceProperties(physical_device, &properties);
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &features);
	
	vulkan_11_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

	vulkan_12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vulkan_12_features.pNext = &vulkan_11_features;

	vulkan_13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	vulkan_13_features.pNext = &vulkan_12_features;

	VkPhysicalDeviceFeatures2 features2 = {};
	features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.pNext = &vulkan_13_features;
	vkGetPhysicalDeviceFeatures2(physical_device, &features2);

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	queue_family_properties.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

	uint32_t available_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(available_extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, available_extensions.data());
	for (const auto& available_extension : available_extensions)
		this->supported_extensions.emplace(available_extension.extensionName, available_extension.specVersion);

	// Try to find a queue family index that supports compute but not graphics
	for (uint32_t i = 0; i < queue_family_properties.size(); i++)
	{
		if ((queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
		{
			compute_queue = i;
			queue_family_indices.emplace_back(i);
			break;
		}
	}

	// Try to find a queue family index that supports transfer but not graphics
	for (uint32_t i = 0; i < queue_family_properties.size(); i++)
	{
		if ((queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
		{
			transfer_queue = i;
			queue_family_indices.emplace_back(i);
			break;
		}
	}

	// Try to find a graphics queue family
	for (size_t i = 0; i < queue_family_properties.size(); ++i)
	{
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphics_queue = i;
			queue_family_indices.emplace_back((uint32_t) i);
			break;
		}
	}

	// Try to find compute/graphics combined queue family, if compute only queue family does not exist
	if (compute_queue == -1)
	{
		for (size_t i = 0; i < queue_family_properties.size(); ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				compute_queue = i;
				queue_family_indices.emplace_back(i);
				break;
			}
		}
	}

	// Try to find compute/graphics combined queue family, if compute only queue family does not exist
	if (transfer_queue == -1)
	{
		for (size_t i = 0; i < queue_family_properties.size(); ++i)
		{
			if (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				transfer_queue = i;
				queue_family_indices.emplace_back(i);
			}
		}
	}

	std::ranges::sort(queue_family_indices); // If you remove this make sure to add it in logical device

	VkSampleCountFlags sample_count = properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts;
	if (sample_count & VK_SAMPLE_COUNT_64_BIT) max_usable_sample_count = VK_SAMPLE_COUNT_64_BIT;
	else if (sample_count & VK_SAMPLE_COUNT_32_BIT) max_usable_sample_count = VK_SAMPLE_COUNT_32_BIT;
	else if (sample_count & VK_SAMPLE_COUNT_16_BIT) max_usable_sample_count = VK_SAMPLE_COUNT_16_BIT;
	else if (sample_count & VK_SAMPLE_COUNT_8_BIT) max_usable_sample_count = VK_SAMPLE_COUNT_8_BIT;
	else if (sample_count & VK_SAMPLE_COUNT_4_BIT) max_usable_sample_count = VK_SAMPLE_COUNT_4_BIT;
	else if (sample_count & VK_SAMPLE_COUNT_2_BIT) max_usable_sample_count = VK_SAMPLE_COUNT_2_BIT;
	else max_usable_sample_count = VK_SAMPLE_COUNT_1_BIT;
	depth_format = findSupportedFormat({ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool PhysicalDevice::supportsExtension(const char* extension_name) const 
{ 
	return supported_extensions.contains(extension_name); 
}

bool PhysicalDevice::supportsFeature(PhysicalDevice::Feature feature) const
{
	constexpr size_t off = (sizeof(VkStructureType) + sizeof(void*)) / sizeof(VkBool32);
	if (feature <= Feature::VULKAN10_LAST)
		return *(((const VkBool32*)&features) + ecast(feature));
	if (feature <= Feature::VULKAN11_LAST)
		return *(((const VkBool32*)&vulkan_11_features) + off + (ecast(feature) - ecast(Feature::VULKAN10_LAST) - 1));
	if (feature <= Feature::VULKAN12_LAST)
		return *(((const VkBool32*)&vulkan_12_features) + off + (ecast(feature) - ecast(Feature::VULKAN11_LAST) - 1));
	if (feature <= Feature::VULKAN13_LAST)
		return *(((const VkBool32*)&vulkan_13_features) + off + (ecast(feature) - ecast(Feature::VULKAN12_LAST) - 1));
	return false;
}

VkDevice PhysicalDevice::createLogicalDevice(const VkDeviceCreateInfo& create_info) const
{
	VkDevice device = nullptr;
	RenderContext::vulkanAssert(vkCreateDevice(physical_device, &create_info, nullptr, &device));
	return device;
}

VkFormatProperties PhysicalDevice::getFormatProperties(VkFormat format) const
{
	VkFormatProperties format_properties{};
	vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);
	return format_properties;
}

VkImageFormatProperties PhysicalDevice::getImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const
{
	VkImageFormatProperties image_format_properties{};
	vkGetPhysicalDeviceImageFormatProperties(physical_device, format, type, tiling, usage, flags, &image_format_properties);
	return image_format_properties;
}

VkFormat PhysicalDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	if (tiling == VK_IMAGE_TILING_OPTIMAL)
		for (VkFormat format : candidates)
			if ((getFormatProperties(format).optimalTilingFeatures & features) == features)
				return format;

	if (tiling == VK_IMAGE_TILING_LINEAR)
		for (VkFormat format : candidates)
			if ((getFormatProperties(format).linearTilingFeatures & features) == features)
				return format;

	SK_ERROR("Vulkan: Couldn't find supported format");
	return VkFormat(0);
}

std::vector<VkSurfaceFormatKHR> PhysicalDevice::getSurfaceFormats(VkSurfaceKHR surface) const
{
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());
	return formats;
}

std::vector<VkPresentModeKHR> PhysicalDevice::getSurfacePresentModes(VkSurfaceKHR surface) const
{
	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
	return present_modes;
}

VkSurfaceCapabilitiesKHR PhysicalDevice::getSurfaceCapabilities(VkSurfaceKHR surface) const
{
	VkSurfaceCapabilitiesKHR capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
	return capabilities;
}

bool PhysicalDevice::getSurfaceSupport(uint32_t queue_family_index, VkSurfaceKHR surface) const
{
	VkBool32 supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface, &supported);
	return supported;
}