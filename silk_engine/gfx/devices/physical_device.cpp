#include "physical_device.h"
#include "logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/render_context.h"
#include "gfx/window/surface.h"
#include "gfx/window/window.h"
#include "gfx/instance.h"

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device)
	: physical_device(physical_device)
{
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

PhysicalDevice* PhysicalDevice::select(VkSurfaceKHR surface)
{
	int max_score = -1;
	unique<PhysicalDevice> best_physical_device = nullptr;
	for (const auto& vk_physical_device : RenderContext::getInstance().getAvailablePhysicalDevices())
	{
		int score = -1;
		unique<PhysicalDevice> physical_device = makeUnique<PhysicalDevice>(vk_physical_device);

		if (physical_device->graphics_queue == -1)
			continue;

		for (const auto& required_extension : LogicalDevice::getRequiredExtensions())
			if (!physical_device->supported_extensions.contains(required_extension))
				continue;

		for (const auto& preferred_extension : LogicalDevice::getPreferredExtensions())
			if (physical_device->supported_extensions.contains(preferred_extension))
				continue;

		// TODO:
		if (surface)
		{
			bool supports_surface = true;
			if (!supports_surface)
				continue;
		}

		score = 0;

		const VkPhysicalDeviceProperties& properties = physical_device->getProperties();
		const VkPhysicalDeviceFeatures& features = physical_device->getFeatures();

		score += features.multiDrawIndirect * 1200;
		score += features.samplerAnisotropy * 900;
		score += (physical_device->compute_queue != -1) * 700;
		score += (physical_device->transfer_queue != -1) * 300;
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

		if (score > max_score)
		{
			max_score = score;
			best_physical_device = std::move(physical_device);
		}
	}

#ifdef SK_ENABLE_DEBUG_OUTPUT
	if (best_physical_device)
	{
		std::string device_type = "Other";
		switch (best_physical_device->getProperties().deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: device_type = "Integrated GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: device_type = "Discrete GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: device_type = "Virtual GPU"; break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU: device_type = "CPU"; break;
		}
		SK_INFO("{}: {}", device_type, best_physical_device->getProperties().deviceName);
	}
#endif

	return best_physical_device.release();
}