#include "physical_device.h"
#include "logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/graphics.h"
#include "gfx/window/surface.h"
#include "gfx/instance.h"

bool QueueFamilyIndices::isSuitable() const
{
	return graphics.has_value()
		&& transfer.has_value()
		&& present.has_value()
		&& compute.has_value();
}

std::vector<uint32_t> QueueFamilyIndices::getIndices() const
{
	if (!isSuitable())
		return {};

	return { *graphics, *transfer, *present, *compute };
}

PhysicalDevice::PhysicalDevice()
{
	physical_device = chooseMostSuitablePhysicalDevice(Graphics::instance->getAvailablePhysicalDevices());
	vkGetPhysicalDeviceProperties(physical_device, &properties);
	vkGetPhysicalDeviceFeatures(physical_device, &features);
	queue_family_indices = findQueueFamilies(physical_device);
	updateSurfaceDetails();
	max_usable_sample_count = getMaxSampleCount(properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts);
	depth_format = findDepthFormat();
	stencil_format = findStencilFormat();
}

VkDevice PhysicalDevice::createLogicalDevice(const VkDeviceCreateInfo& create_info) const
{
	VkDevice device = nullptr;
	Graphics::vulkanAssert(vkCreateDevice(physical_device, &create_info, nullptr, &device));
	return device;
}
QueueFamilyIndices PhysicalDevice::findQueueFamilies(VkPhysicalDevice physical_device)
{
	QueueFamilyIndices queue_family_indices = {};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

	for (uint32_t i = 0; i < queue_family_properties.size(); ++i)
	{
		const auto& queue_family = queue_family_properties[i];
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			queue_family_indices.graphics = i;
		
		if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
			queue_family_indices.compute = i;

		if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT)
			queue_family_indices.transfer = i;

		VkBool32 supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, *Graphics::surface, &supported);
		if (supported)
			queue_family_indices.present = i;

		if (queue_family_indices.isSuitable())
			break;
	}

	return queue_family_indices;
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

void PhysicalDevice::updateSurfaceCapabilities()
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *Graphics::surface, &surface_capabilities);
}

void PhysicalDevice::updateSurfaceDetails()
{
	updateSurfaceCapabilities();

	uint32_t surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *Graphics::surface, &surface_format_count, nullptr);
	surface_formats.resize(surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *Graphics::surface, &surface_format_count, surface_formats.data());

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *Graphics::surface, &present_mode_count, nullptr);
	present_modes.resize(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *Graphics::surface, &present_mode_count, present_modes.data());
}

VkFormat PhysicalDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props = getFormatProperties(format);
		if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features
			|| (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)))
			return format;
	}

	SK_ERROR("Vulkan: Couldn't find supported format");
	return VkFormat(0);
}

VkFormat PhysicalDevice::findDepthFormat() const
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

VkFormat PhysicalDevice::findStencilFormat() const
{
	using enum VkFormat;
	return findSupportedFormat
	(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
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

	SK_ASSERT(candidates.rbegin()->first >= 0, 
		"Vulkan: Couldn't find suitable vulkan GPU");

	return candidates.rbegin()->second;
}

// This function chooses the "best" gpu
int PhysicalDevice::ratePhysicalDevice(VkPhysicalDevice physical_device)
{
	int score = 0;

	QueueFamilyIndices queue_family_indices = findQueueFamilies(physical_device);
	if (!queue_family_indices.isSuitable()) 
		return -1;

	bool extensions_supported = checkPhysicalDeviceExtensionSupport(LogicalDevice::getRequiredExtensions(), physical_device);
	if (!extensions_supported) 
		return -1;

	VkSurfaceCapabilitiesKHR surface_capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, *Graphics::surface, &surface_capabilities);

	uint32_t surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *Graphics::surface, &surface_format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, *Graphics::surface, &surface_format_count, surface_formats.data());

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *Graphics::surface, &present_mode_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, *Graphics::surface, &present_mode_count, present_modes.data());

	bool is_swap_chain_adequate = !surface_formats.empty() &&
		!present_modes.empty();
	if (!is_swap_chain_adequate) 
		return -1;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physical_device, &properties);
	VkPhysicalDeviceFeatures features{};
	vkGetPhysicalDeviceFeatures(physical_device, &features);

	//TODO: add critical feature dependencies as engine needs it
	if (!(features.multiDrawIndirect && features.samplerAnisotropy))
		return -1;

	score += (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 500;
	score += features.multiDrawIndirect * 250;
	score += features.geometryShader * 200;
	score += features.tessellationShader * 100;
	score += features.fragmentStoresAndAtomics * 50;
	score += features.samplerAnisotropy * 50;
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

bool PhysicalDevice::checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, VkPhysicalDevice physical_device)
{
	uint32_t available_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(available_extension_count);
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, available_extensions.data());

	for (const auto& required_extension : required_extensions)
	{
		bool extension_found = false;
		for (const auto& available_extension : available_extensions)
			if (strcmp(required_extension, available_extension.extensionName) == 0)
				extension_found = true;
		if (!extension_found)
			return false;
	}

	return true;
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