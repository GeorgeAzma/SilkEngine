#include "image.h"
#include "gfx/graphics.h"
#include "gfx/buffers/buffer.h"
#include "gfx/enums.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"
#include "gfx/window/swap_chain.h"

Image::~Image()
{
	view = nullptr;
	sampler = nullptr;
	if(allocation != VK_NULL_HANDLE)
		vmaDestroyImage(*Graphics::allocator, image, allocation);
}

void Image::align4(Bitmap& image)
{
	int old_channels = image.channels;
	image.channels = 4;

	size_t size = image.width * image.height;
	std::vector<uint8_t> aligned_image(size * 4, 0);
	for (size_t i = 0; i < size; ++i)
	{
		std::memcpy(aligned_image.data() + i * 4, image.data.data() + i * old_channels, old_channels * sizeof(uint8_t));
		aligned_image[i * 4 + 3] = 255;
	}
	image.data = std::move(aligned_image);
}

void Image::create(const ImageProps& props)
{
	needs_staging = EnumInfo::needsStaging(props.memory_usage);
	SK_ASSERT((props.is_1D ? (props.height == 1 && props.depth == 1) : true), "If image is 1D it's height and depth must be 1");
	SK_ASSERT(((!props.is_1D && ! props.is_cubemap) ? (props.depth == 1) : true), "If image is 2D it's depth must be 1");
	unique<StagingBuffer> staging_buffer = props.data ? makeUnique<StagingBuffer>(props.data, props.width * props.height * EnumInfo::formatSize(props.format) * props.array_layers) : nullptr;
	descriptor_image_info.imageLayout = props.initial_layout;

	if (image == VK_NULL_HANDLE)
	{
#ifdef SK_ENABLE_DEBUG_OUTPUT
		if (props.usage & VK_IMAGE_USAGE_STORAGE_BIT)
		{
			VkFormatProperties format_properties;
			vkGetPhysicalDeviceFormatProperties(*Graphics::physical_device, props.format, &format_properties);
			SK_ASSERT(((props.tiling == VK_IMAGE_TILING_OPTIMAL) ? format_properties.optimalTilingFeatures : format_properties.linearTilingFeatures) & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT, "Storage image doesn't support specified format.");
		}
#endif
		VkImageCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.imageType = props.is_1D ? VK_IMAGE_TYPE_1D : (props.depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D);
		create_info.format = props.format;
		create_info.tiling = props.tiling;
		create_info.usage = props.usage;
		create_info.flags = props.is_cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
		create_info.initialLayout = props.initial_layout;
		create_info.extent = { props.width, props.height, props.depth };
		create_info.arrayLayers = props.array_layers;
		if (props.mipmap)
		{
			mip_levels = std::floor(std::log2(std::max(props.width, props.height))) + 1;
			auto max_mip_levels = Graphics::physical_device->getImageFormatProperties(create_info.format, create_info.imageType, create_info.tiling, create_info.usage, create_info.flags).maxMipLevels;
		}
		create_info.mipLevels = mip_levels;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.samples = props.samples;

		VmaAllocationCreateInfo allocation_info = {};
		allocation_info.usage = props.memory_usage;

		Graphics::vulkanAssert(vmaCreateImage(*Graphics::allocator, &create_info, &allocation_info, &image, &allocation, nullptr));
	}

	if (staging_buffer.get() != nullptr)
	{
		transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyFromBuffer(*staging_buffer);
		if (props.mipmap)
			generateMipmaps();
	}

	transitionLayout(props.layout); 
	
	if (props.create_view)
	{
		view = makeUnique<ImageView>(this->image, props.format, mip_levels, props.array_layers, (props.is_cubemap ? ImageViewType::CUBEMAP : (props.is_1D ? ImageViewType::IMAGE1D : (props.depth == 1 ? ImageViewType::IMAGE2D : ImageViewType::IMAGE3D))));
		descriptor_image_info.imageView = *view;
	}
	if (props.create_sampler)
	{
		SamplerProps sampler_props = props.sampler_props;
		sampler_props.mip_levels = mip_levels;
		sampler_props.linear_mipmap = props.mipmap_filter != VK_FILTER_NEAREST;
		sampler = makeUnique<Sampler>(sampler_props);
		descriptor_image_info.sampler = *sampler;
	}
}

void Image::transitionLayout(VkImageLayout new_layout) 
{
	if (descriptor_image_info.imageLayout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;

	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = descriptor_image_info.imageLayout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = getAspectFlags();
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = props.array_layers;

	// Source access mask controls actions that have to be finished on the old layout before it will be transitioned to the new layout.
	switch (barrier.oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		SK_ERROR("Unsupported image layout transition source: {0}", barrier.oldLayout);
		break;
	}

	// Destination access mask controls the dependency for the new image layout.
	switch (new_layout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (barrier.srcAccessMask == 0)
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		SK_ERROR("Unsupported image layout transition destination: {0}", barrier.oldLayout);
		break;
	}

	vkCmdPipelineBarrier(Graphics::active.command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	command_buffer.submitIdle();

	descriptor_image_info.imageLayout = new_layout;
}

void Image::insertMemoryBarrier(VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, uint32_t base_mip_level, uint32_t base_array_layer)
{
	if (descriptor_image_info.imageLayout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;
	insertMemoryBarrier(image, source_access_mask, destination_access_mask, old_layout, new_layout, source_stage_mask, destination_stage_mask, getAspectFlags(), mip_levels, base_mip_level, props.array_layers, base_array_layer);
	descriptor_image_info.imageLayout = new_layout;
}

bool Image::isFeatureSupported(VkFormatFeatureFlags feature) const
{
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(*Graphics::physical_device, props.format, &format_properties);
	const auto& features = (props.tiling == VK_IMAGE_TILING_OPTIMAL) ? format_properties.optimalTilingFeatures : format_properties.linearTilingFeatures;
	return (features & feature);
}

void Image::copyFromBuffer(VkBuffer buffer)
{
	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	size_t offset = 0;
	std::vector<VkBufferImageCopy> copy_regions(props.array_layers);
	for (size_t layer = 0; layer < props.array_layers; ++layer)
	{
		VkBufferImageCopy copy_region{};
		copy_region.bufferOffset = 0;
		copy_region.bufferRowLength = 0;
		copy_region.bufferImageHeight = 0;
		copy_region.imageSubresource.aspectMask = getAspectFlags();
		copy_region.imageSubresource.mipLevel = 0;
		copy_region.imageSubresource.baseArrayLayer = layer;
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageOffset = { 0, 0, 0 };
		copy_region.imageExtent = { props.width, props.height, props.depth };

		offset += props.width * props.height * props.depth * EnumInfo::formatSize(props.format);

		copy_regions[layer] = std::move(copy_region);
	}

	vkCmdCopyBufferToImage(command_buffer, buffer, image, descriptor_image_info.imageLayout, copy_regions.size(), copy_regions.data());

	command_buffer.submitIdle();
}

void Image::insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t array_layers, uint32_t base_array_layer)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = source_access_mask;
	barrier.dstAccessMask = destination_access_mask;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseMipLevel = base_mip_level;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = base_array_layer;
	barrier.subresourceRange.layerCount = array_layers;
	vkCmdPipelineBarrier(Graphics::active.command_buffer, source_stage_mask, destination_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Image::generateMipmaps()
{
	if (mip_levels <= 1)
		return;

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(*Graphics::physical_device, props.format, &format_properties);

	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
		"Image format does not support linear blitting");
	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
		"Image format does not support src blitting");
	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
		"Image format does not support dst blitting");

	CommandBuffer command_buffer{};
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = getAspectFlags();
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = props.array_layers;
	barrier.subresourceRange.levelCount = 1;

	int32_t mip_width = props.width;
	int32_t mip_height = props.height;

	for (uint32_t i = 1; i < mip_levels; ++i)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.subresourceRange.baseMipLevel = i - 1;
		vkCmdPipelineBarrier(Graphics::active.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = props.array_layers;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = 
		{ 
			mip_width > 1 ? mip_width / 2 : 1, 
			mip_height > 1 ? mip_height / 2 : 1, 1 
		};
		blit.dstSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = props.array_layers;

		vkCmdBlitImage(Graphics::active.command_buffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			props.mipmap_filter);

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = props.layout;
		descriptor_image_info.imageLayout = barrier.newLayout;
		vkCmdPipelineBarrier(Graphics::active.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mip_width > 1) 
			mip_width /= 2;

		if (mip_height > 1) 
			mip_height /= 2;
	}

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = props.layout;
	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	vkCmdPipelineBarrier(Graphics::active.command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	command_buffer.submitIdle();
}

void Image::setData(void* data, uint32_t array_layers, uint32_t base_array_layer)
{
	VkImageSubresource image_subresource = {};
	image_subresource.aspectMask = getAspectFlags();
	image_subresource.mipLevel = 0;
	image_subresource.arrayLayer = base_array_layer;
	VkSubresourceLayout subresource_layout;
	vkGetImageSubresourceLayout(*Graphics::logical_device, image, &image_subresource, &subresource_layout);

	if (needs_staging)
	{
		StagingBuffer staging_buffer(data, subresource_layout.size);
		void* buffer_data;
		staging_buffer.map(&buffer_data);
		memcpy((uint8_t*)buffer_data + subresource_layout.offset, data, staging_buffer.size);
		staging_buffer.unmap();
		copyFromBuffer(staging_buffer);
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy((uint8_t*)buffer_data + subresource_layout.offset, data, subresource_layout.size);
		unmap();
	}
}

void Image::getData(void* data, uint32_t array_layer)
{
	VkImageSubresource image_subresource = {};
	image_subresource.aspectMask = getAspectFlags();
	image_subresource.mipLevel = 0;
	image_subresource.arrayLayer = array_layer;
	VkSubresourceLayout subresource_layout;
	vkGetImageSubresourceLayout(*Graphics::logical_device, image, &image_subresource, &subresource_layout);
	
	if (needs_staging)
	{
		SK_TODO("");
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy(data, (const uint8_t*)buffer_data + subresource_layout.offset, subresource_layout.size);
		unmap();
	}
}

bool Image::copyImage(shared<Image> destination, uint32_t array_layer)
{
	bool supports_blit = true;
	
	if(!isFeatureSupported(VK_FORMAT_FEATURE_BLIT_SRC_BIT))
	{
		SK_WARN("Device does not support blitting from optimal tiled images, using copy instead of blit!");
		supports_blit = false;
	}

	if (!destination->isFeatureSupported(VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		SK_WARN("Device does not support blitting to linear tiled images, using copy instead of blit!\n");
		supports_blit = false;
	}

	//Do the actual blit from the swapchain image to our host visible destination image.
	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//Transition destination image to transfer destination layout.
	auto destination_old_layout = destination->getDescriptorInfo().imageLayout;
	destination->insertMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, destination_old_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

	//Transition image from previous usage to transfer source layout
	auto old_layout = descriptor_image_info.imageLayout;
	insertMemoryBarrier(VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, old_layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, array_layer);

	//If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
	if (supports_blit) 
	{
		//Define the region to blit (we will blit the whole swapchain image).
		VkOffset3D blit_size = { int32_t(props.width), int32_t(props.height), int32_t(props.depth) };

		VkImageBlit image_blit_region = {};
		image_blit_region.srcSubresource.aspectMask = getAspectFlags();
		image_blit_region.srcSubresource.mipLevel = 0;
		image_blit_region.srcSubresource.baseArrayLayer = array_layer;
		image_blit_region.srcSubresource.layerCount = 1;
		image_blit_region.srcOffsets[0] = { 0, 0, 0 };
		image_blit_region.srcOffsets[1] = blit_size;
		image_blit_region.dstSubresource.aspectMask = destination->getAspectFlags();
		image_blit_region.dstSubresource.mipLevel = 0;
		image_blit_region.dstSubresource.baseArrayLayer = 0;
		image_blit_region.dstSubresource.layerCount = 1;
		image_blit_region.dstOffsets[0] = { 0, 0, 0 };
		image_blit_region.dstOffsets[1] = blit_size;
		vkCmdBlitImage(Graphics::active.command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_blit_region, VK_FILTER_NEAREST);
	}
	else 
	{
		//Otherwise use image copy (requires us to manually flip components).
		VkImageCopy image_copy_region{};
		image_copy_region.srcSubresource.aspectMask = getAspectFlags();
		image_copy_region.srcSubresource.mipLevel = 0;
		image_copy_region.srcSubresource.baseArrayLayer = array_layer;
		image_copy_region.srcSubresource.layerCount = 1;
		image_copy_region.dstSubresource.aspectMask = destination->getAspectFlags();
		image_copy_region.dstSubresource.mipLevel = 0;
		image_copy_region.dstSubresource.baseArrayLayer = 0;
		image_copy_region.dstSubresource.layerCount = 1;
		image_copy_region.extent = { props.width, props.height, props.depth };
		vkCmdCopyImage(Graphics::active.command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy_region);
	}

	//Transition destination image to general layout, which is the required layout for mapping the image memory later on.
	destination->insertMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, destination_old_layout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);
	
	//Transition back the image after the blit is done.
	insertMemoryBarrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, old_layout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, array_layer);

	command_buffer.submitIdle();

	return supports_blit;
}

void Image::map(void** data) const
{
	if (mapped)
		return;
	vmaMapMemory(*Graphics::allocator, allocation, data);
	mapped = true;
}

void Image::unmap() const
{
	if (!mapped)
		return;
	vmaUnmapMemory(*Graphics::allocator, allocation);
	mapped = false;
}

VkFormat Image::getDefaultFormatFromChannelCount(int channels)
{
	switch (channels)
	{
	case 0: return VkFormat(0);
	case 1: return VK_FORMAT_R8_UNORM;
	case 2: return VK_FORMAT_R8G8_UNORM;
	case 3: return VK_FORMAT_R8G8B8_UNORM;
	case 4: return VK_FORMAT_R8G8B8A8_UNORM;
	}

	SK_ERROR("Unsupported channel count specified: {0}", channels);
	return VkFormat(0);
}

bool Image::hasStencil(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT
		|| format == VK_FORMAT_D16_UNORM_S8_UINT
		|| format == VK_FORMAT_S8_UINT;
}

bool Image::hasDepth(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT 
		|| format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT
		|| format == VK_FORMAT_D16_UNORM_S8_UINT;
}

VkImageAspectFlags Image::getAspectFlags(VkFormat format)
{
	if (!hasDepth(format))
		return VK_IMAGE_ASPECT_COLOR_BIT;

	if (hasStencil(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	return VK_IMAGE_ASPECT_DEPTH_BIT;
}

size_t Image::channelCount(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8_SINT: return 1;
	case VK_FORMAT_R8_UINT: return 1;
	case VK_FORMAT_R16_SINT: return 1;
	case VK_FORMAT_R16_UINT: return 1;
	case VK_FORMAT_R32_SINT: return 1;
	case VK_FORMAT_R32_UINT: return 1;
	case VK_FORMAT_R32_SFLOAT: return 1;
	case VK_FORMAT_R64_SFLOAT: return 1;
	case VK_FORMAT_R32G32_SFLOAT: return 2;
	case VK_FORMAT_R32G32B32_SFLOAT: return 3;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return 4;
	case VK_FORMAT_R32G32_SINT: return 2;
	case VK_FORMAT_R32G32B32_SINT: return 3;
	case VK_FORMAT_R32G32B32A32_SINT: return 4;
	case VK_FORMAT_R32G32_UINT: return 2;
	case VK_FORMAT_R32G32B32_UINT:  return 3;
	case VK_FORMAT_R32G32B32A32_UINT: return 4;
	case VK_FORMAT_R64G64_SFLOAT: return 2;
	case VK_FORMAT_R64G64B64_SFLOAT: return 3;
	case VK_FORMAT_R64G64B64A64_SFLOAT: return 4;
	case VK_FORMAT_R8_SRGB: return 1;
	case VK_FORMAT_R8G8_SRGB: return 2;
	case VK_FORMAT_R8G8B8_SRGB: return 3;
	case VK_FORMAT_R8G8B8A8_SRGB: return 4;
	case VK_FORMAT_R8_UNORM: return 1;
	case VK_FORMAT_R8G8_UNORM: return 2;
	case VK_FORMAT_R8G8B8_UNORM: return 3;
	case VK_FORMAT_R8G8B8A8_UNORM: return 4;
	case VK_FORMAT_D16_UNORM: return 1;
	case VK_FORMAT_D16_UNORM_S8_UINT: return 1;
	case VK_FORMAT_D24_UNORM_S8_UINT: return 1;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: return 1;
	case VK_FORMAT_D32_SFLOAT: return 1;
	case VK_FORMAT_B8G8R8_SRGB: return 3;
	case VK_FORMAT_B8G8R8A8_SRGB: return 4;
	case VK_FORMAT_B8G8R8_UNORM: return 3;
	case VK_FORMAT_B8G8R8A8_UNORM: return 4;
	}

	SK_ERROR("Unsupported format specified: {0}.", format);
	return 0;
}