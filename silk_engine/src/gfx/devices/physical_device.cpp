#include "physical_device.h"
#include "logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/graphics.h"
#include "gfx/window/surface.h"
#include "gfx/instance.h"

PhysicalDevice::PhysicalDevice()
{
	physical_device = chooseMostSuitablePhysicalDevice(Graphics::instance->getAvailablePhysicalDevices());
	properties = physical_device.getProperties();
	features = physical_device.getFeatures();
	queue_family_indices = findQueueFamilies(physical_device);
	updateSurfaceDetails();
	max_usable_sample_count = getMaxSampleCount(properties.limits.framebufferColorSampleCounts & properties.limits.framebufferDepthSampleCounts);
	depth_format = findDepthFormat();
	stencil_format = findStencilFormat();
}

vk::Device PhysicalDevice::createLogicalDevice(const vk::DeviceCreateInfo& create_info) const
{
	return physical_device.createDevice(create_info);
}
QueueFamilyIndices PhysicalDevice::findQueueFamilies(vk::PhysicalDevice physical_device)
{
	QueueFamilyIndices queue_family_indices = {};

	auto queue_families = physical_device.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queue_families.size(); ++i)
	{
		const auto& queue_family = queue_families[i];
		if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
			queue_family_indices.graphics = i;
		
		if (queue_family.queueFlags & vk::QueueFlagBits::eCompute)
			queue_family_indices.compute = i;

		if (queue_family.queueFlags & vk::QueueFlagBits::eTransfer)
			queue_family_indices.transfer = i;

		if (physical_device.getSurfaceSupportKHR(i, *Graphics::surface))
			queue_family_indices.present = i;

		if (queue_family_indices.isSuitable())
			break;
	}

	return queue_family_indices;
}

vk::FormatProperties PhysicalDevice::getFormatProperties(vk::Format format) const
{
	return physical_device.getFormatProperties(format);
}

vk::ImageFormatProperties PhysicalDevice::getImageFormatProperties(vk::Format format, vk::ImageType type, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageCreateFlags flags) const
{
	return physical_device.getImageFormatProperties(format, type, tiling, usage, flags);
}

void PhysicalDevice::updateSurfaceCapabilities()
{
	surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*Graphics::surface);
}

void PhysicalDevice::updateSurfaceDetails()
{
	surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*Graphics::surface);
	surface_formats = physical_device.getSurfaceFormatsKHR(*Graphics::surface);
	present_modes = physical_device.getSurfacePresentModesKHR(*Graphics::surface);
}

vk::Format PhysicalDevice::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties props = getFormatProperties(format);
		if ((tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features
			|| (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)))
			return format;
	}

	SK_ERROR("Vulkan: Couldn't find supported format");
	return vk::Format(0);
}

vk::Format PhysicalDevice::findDepthFormat() const
{
	using enum vk::Format;
	return findSupportedFormat
	(
		{ eD32Sfloat,
		eD32SfloatS8Uint,
		eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

vk::Format PhysicalDevice::findStencilFormat() const
{
	using enum vk::Format;
	return findSupportedFormat
	(
		{ eD32SfloatS8Uint,
		eD24UnormS8Uint,
		eS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

vk::PhysicalDevice PhysicalDevice::chooseMostSuitablePhysicalDevice(const std::vector<vk::PhysicalDevice>& physical_devices)
{
	std::multimap<int, vk::PhysicalDevice> candidates;
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
int PhysicalDevice::ratePhysicalDevice(vk::PhysicalDevice physical_device)
{
	int score = 0;

	QueueFamilyIndices queue_family_indices = findQueueFamilies(physical_device);
	if (!queue_family_indices.isSuitable()) 
		return -1;

	bool extensions_supported = checkPhysicalDeviceExtensionSupport(LogicalDevice::getRequiredExtensions(), physical_device);
	if (!extensions_supported) 
		return -1;

	auto surface_capabilities = physical_device.getSurfaceCapabilitiesKHR(*Graphics::surface);
	auto surface_formats = physical_device.getSurfaceFormatsKHR(*Graphics::surface);
	auto present_modes = physical_device.getSurfacePresentModesKHR(*Graphics::surface);

	bool is_swap_chain_adequate = !surface_formats.empty() &&
		!present_modes.empty();
	if (!is_swap_chain_adequate) 
		return -1;

	auto properties = physical_device.getProperties();
	auto features = physical_device.getFeatures();

	//TODO: add critical feature dependencies as engine needs it
	if (!(features.multiDrawIndirect && features.samplerAnisotropy))
		return -1;

	score += (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) * 500;
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

bool PhysicalDevice::checkPhysicalDeviceExtensionSupport(const std::vector<const char*>& required_extensions, vk::PhysicalDevice physical_device)
{
	auto available_extensions = physical_device.enumerateDeviceExtensionProperties();

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

vk::SampleCountFlagBits PhysicalDevice::getMaxSampleCount(vk::SampleCountFlags counts)
{
	using enum vk::SampleCountFlagBits;
	if (counts & e64) return e64;
	if (counts & e32) return e32;
	if (counts & e16) return e16;
	if (counts & e8) return e8;
	if (counts & e4) return e4;
	if (counts & e2) return e2;
	return e1;
}