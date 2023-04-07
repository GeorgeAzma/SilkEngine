#include "physical_device.h"
#include "logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/render_context.h"
#include "gfx/window/surface.h"
#include "gfx/window/window.h"
#include "gfx/instance.h"

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device)
{
	init(physical_device);
}

PhysicalDevice::PhysicalDevice()
{
	init(chooseMostSuitablePhysicalDevice(RenderContext::getInstance().getAvailablePhysicalDevices()));

#ifdef SK_ENABLE_DEBUG_OUTPUT
	std::string device_type = "Other";
	switch (properties.deviceType)
	{
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: device_type = "Integrated GPU"; break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: device_type = "Discrete GPU"; break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: device_type = "Virtual GPU"; break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU: device_type = "CPU"; break;
	}
	SK_INFO("{}: {}", device_type, properties.deviceName);
#endif
}

bool PhysicalDevice::supportsExtension(const char* extension_name) const 
{ 
	return supported_extensions.contains(extension_name); 
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

void PhysicalDevice::init(VkPhysicalDevice physical_device)
{
	this->physical_device = physical_device;
	vkGetPhysicalDeviceProperties(physical_device, &properties);
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
	vkGetPhysicalDeviceFeatures(physical_device, &features);

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
			queue_family_indices.emplace_back(i);
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

	max_usable_sample_count = getMaxSampleCount(properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts);
}

VkFormat PhysicalDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props = getFormatProperties(format);
		if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) || 
			(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features))
			return format;
	}

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

VkPhysicalDevice PhysicalDevice::chooseMostSuitablePhysicalDevice(const std::vector<VkPhysicalDevice>& physical_devices)
{
	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : physical_devices)
	{
		int score = ratePhysicalDevice(device);
		if(score >= 0)
			candidates.insert(std::make_pair(score, device));
	}

	SK_ASSERT(candidates.size() > 0, "Vulkan: Couldn't find suitable vulkan GPU");

	return candidates.rbegin()->second;
}

// This function chooses the "best" gpu
int PhysicalDevice::ratePhysicalDevice(VkPhysicalDevice physical_device)
{
	int score = 0;
	
	PhysicalDevice pd(physical_device);
	
	if (pd.graphics_queue == -1)
		return -1;

	for (const auto& required_extension : LogicalDevice::getRequiredExtensions())
		if (!pd.supported_extensions.contains(required_extension))
			return -1;

	for (const auto& preferred_extension : LogicalDevice::getPreferredExtensions())
		if (pd.supported_extensions.contains(preferred_extension))
			score += 50;

	// TODO:
	//bool is_swap_chain_adequate = !surface_formats.empty() && !present_modes.empty();
	//if (!is_swap_chain_adequate) 
	//	return -1;

	const VkPhysicalDeviceProperties& properties = pd.getProperties();
	const VkPhysicalDeviceFeatures& features = pd.getFeatures();

	score += features.multiDrawIndirect * 1200;
	score += features.samplerAnisotropy * 900;
	score += (pd.compute_queue != -1) * 700;
	score += (pd.transfer_queue != -1) * 300;
	score += (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 500;
	score += features.multiDrawIndirect * 250;
	score += features.geometryShader * 200;
	score += features.tessellationShader * 100;
	score += features.samplerAnisotropy * 80;
	score += features.fragmentStoresAndAtomics * 50;
	score += features.vertexPipelineStoresAndAtomics * 30;
	score += features.shaderSampledImageArrayDynamicIndexing * 25;
	score += features.shaderStorageBufferArrayDynamicIndexing * 25;
	score += features.shaderStorageImageArrayDynamicIndexing * 25;
	score += features.shaderUniformBufferArrayDynamicIndexing * 25;
	score += features.occlusionQueryPrecise * 25;
	score += features.multiViewport * 25;
	score += features.wideLines * 20;
	score += features.largePoints * 10;
	score += features.imageCubeArray * 10;
	score += features.pipelineStatisticsQuery * 5;
	score += features.shaderFloat64 * 5;
	score += features.alphaToOne;
	score += features.depthBiasClamp;
	score += features.depthBounds;
	score += features.depthClamp;
	score += features.drawIndirectFirstInstance;
	score += features.dualSrcBlend;
	score += features.fillModeNonSolid;
	score += features.fullDrawIndexUint32;
	score += features.independentBlend;
	score += features.inheritedQueries;
	score += features.logicOp;
	score += features.robustBufferAccess;
	score += features.sampleRateShading;
	score += features.shaderClipDistance;
	score += features.shaderCullDistance;
	score += features.shaderImageGatherExtended;
	score += features.shaderInt16;
	score += features.shaderInt64;
	score += features.shaderResourceMinLod;
	score += features.shaderResourceResidency;
	score += features.shaderStorageImageExtendedFormats;
	score += features.shaderStorageImageMultisample;
	score += features.shaderStorageImageReadWithoutFormat;
	score += features.shaderStorageImageWriteWithoutFormat;
	score += features.shaderTessellationAndGeometryPointSize;
	score += features.sparseBinding;
	score += features.sparseResidency16Samples;
	score += features.sparseResidency2Samples;
	score += features.sparseResidency4Samples;
	score += features.sparseResidency8Samples;
	score += features.sparseResidency16Samples;
	score += features.sparseResidencyAliased;
	score += features.sparseResidencyBuffer;
	score += features.sparseResidencyImage2D;
	score += features.sparseResidencyImage3D;
	score += features.textureCompressionASTC_LDR;
	score += features.textureCompressionBC;
	score += features.textureCompressionETC2;
	score += features.variableMultisampleRate;

	return score;
}

VkSampleCountFlagBits PhysicalDevice::getMaxSampleCount(VkSampleCountFlags counts)
{
	if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
	return VK_SAMPLE_COUNT_1_BIT;
}