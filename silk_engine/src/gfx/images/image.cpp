#include "image.h"
#include "gfx/graphics.h"
#include "gfx/buffers/buffer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/buffers/storage_buffer.h"
#include "gfx/buffers/command_buffer.h"

Image::~Image()
{
	view = nullptr;
	sampler = nullptr;
	if(allocation != VK_NULL_HANDLE)
		vmaDestroyImage(*Graphics::allocator, image, allocation);
}

void Image::create(const ImageProps& props)
{
	this->props = props;
	needs_staging = Allocator::needsStaging(props.memory_usage);
	SK_ASSERT((props.is_1D ? (props.height == 1 && props.depth == 1) : true), "If image is 1D it's height and depth must be 1");
	SK_ASSERT(((!props.is_1D && ! props.is_cubemap) ? (props.depth == 1) : true), "If image is 2D it's depth must be 1");
	unique<StagingBuffer> staging_buffer = props.data ? makeUnique<StagingBuffer>(props.data, getSize()) : nullptr;
	layout = props.initial_layout;

	if (image == VK_NULL_HANDLE)
	{
		VkImageCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ci.imageType = props.is_1D ? VK_IMAGE_TYPE_1D : (props.depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D);
		ci.format = ImageFormatEnum::toVulkanType(props.format);
		ci.tiling = props.tiling;
		ci.usage = props.usage;
		ci.flags = props.is_cubemap * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		ci.initialLayout = props.initial_layout;
		ci.extent = { props.width, props.height, props.depth };
		ci.arrayLayers = props.layers;
		mip_levels = props.mipmap * std::floor(std::log2(std::max(props.width, props.height))) + 1;
		ci.mipLevels = mip_levels;
		ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.samples = props.samples;

		VmaAllocationCreateInfo allocation_info{};
		allocation_info.usage = props.memory_usage;
		Graphics::vulkanAssert(VkResult(vmaCreateImage(*Graphics::allocator, &ci, &allocation_info, &image, &allocation, nullptr)));
	}
	else
	{
		swap_chain_property = true;
	}
	
	if (staging_buffer.get() != nullptr)
	{
		copyFromBuffer(*staging_buffer);
		if (props.mipmap)
			generateMipmaps();
	}

	transitionLayout(props.layout);
	
	if (props.create_view)
	{
		view = makeUnique<ImageView>(this->image, props.format, mip_levels, props.layers, (props.is_cubemap ? ImageViewType::CUBEMAP : (props.is_1D ? ImageViewType::IMAGE1D : (props.depth == 1 ? ImageViewType::IMAGE2D : ImageViewType::IMAGE3D))));
	}
	if (props.create_sampler)
	{
		SamplerProps sampler_props = props.sampler_props;
		sampler_props.mip_levels = mip_levels;
		sampler_props.linear_mipmap = props.mipmap_filter != VK_FILTER_NEAREST;
		sampler = makeUnique<Sampler>(sampler_props);
	}

	SK_TRACE("Image created: {}x{}x{} with {} layers", props.width, props.height, props.depth, props.layers);
}

void Image::transitionLayout(VkImageLayout new_layout) 
{
	if (layout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;

	CommandBuffer command_buffer;
	command_buffer.begin();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = props.layers;
	barrier.srcAccessMask = VkAccessFlags(0);
	barrier.dstAccessMask = VkAccessFlags(0);

	// TODO: this pipeline barrier code was written by total noob, check if it's correct
	// Source access mask controls actions that have to be finished on the old layout before it will be transitioned to the new layout.
	VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags destination_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	switch (barrier.oldLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			source_stage = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			source_stage = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			source_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		default:
			SK_ERROR("Unsupported image layout transition source: {0}", barrier.oldLayout);
			break;
	}

	// Destination access mask controls the dependency for the new image layout.
	switch (new_layout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		case VK_IMAGE_LAYOUT_GENERAL:
			destination_stage = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			destination_stage = VK_PIPELINE_STAGE_HOST_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		default:
			SK_ERROR("Unsupported image layout transition destination: {0}", barrier.oldLayout);
			break;
	}
	Graphics::getActiveCommandBuffer().pipelineBarrier(source_stage, destination_stage, VkDependencyFlags(0), {}, {}, { barrier });

	command_buffer.submitIdle();

	layout = new_layout;
}

void Image::insertMemoryBarrier(VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, uint32_t base_mip_level, uint32_t base_layer)
{
	if (layout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;
	insertMemoryBarrier(image, source_access_mask, destination_access_mask, layout, new_layout, source_stage_mask, destination_stage_mask, ImageFormatEnum::getVulkanAspectFlags(props.format), mip_levels, base_mip_level, props.layers, base_layer);
	layout = new_layout;
}

bool Image::isFeatureSupported(VkFormatFeatureFlags feature) const
{
	VkFormatProperties format_properties = Graphics::physical_device->getFormatProperties(ImageFormatEnum::toVulkanType(props.format));
	const VkFormatFeatureFlags& features = (props.tiling == VK_IMAGE_TILING_OPTIMAL) ? format_properties.optimalTilingFeatures : format_properties.linearTilingFeatures;
	return bool(features & feature);
}

void Image::copyFromBuffer(VkBuffer buffer)
{
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	CommandBuffer command_buffer;
	command_buffer.begin();

	size_t offset = 0;
	std::vector<VkBufferImageCopy> regions(props.layers);
	for (size_t layer = 0; layer < props.layers; ++layer)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = layer;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = VkOffset3D(0, 0, 0);
		region.imageExtent = VkExtent3D(props.width, props.height, props.depth);

		offset += props.width * props.height * props.depth * ImageFormatEnum::getSize(props.format);

		regions[layer] = std::move(region);
	}
	Graphics::getActiveCommandBuffer().copyBufferToImage(buffer, image, layout, regions);

	command_buffer.submitIdle();
}

void Image::copyToBuffer(VkBuffer buffer, uint32_t base_layer, uint32_t layers)
{
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	CommandBuffer command_buffer;
	command_buffer.begin();

	VkBufferImageCopy region{};
	region.imageOffset = VkOffset3D(0, 0, 0);
	region.imageExtent = VkExtent3D(props.width, props.height, props.depth);
	region.bufferOffset = 0;
	region.bufferImageHeight = props.height;
	region.bufferRowLength = props.width;
	region.imageSubresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
	region.imageSubresource.baseArrayLayer = base_layer;
	region.imageSubresource.layerCount = layers;
	region.imageSubresource.mipLevel = 0;
	Graphics::getActiveCommandBuffer().copyImageToBuffer(image, layout, buffer, { region });

	command_buffer.submitIdle();
}

void Image::reallocate(uint32_t width, uint32_t height, uint32_t depth, uint32_t layers)
{
	if (swap_chain_property || (width == props.width && height == props.height && depth == props.depth && layers == props.layers))
		return;

	width = width ? width : props.width;
	height = height ? height : props.height;
	depth = depth ? depth : props.depth;
	layers = layers ? layers : props.layers;

	props.layers = layers;
	props.depth = depth;
	props.height = height;
	props.width = width;

	vmaDestroyImage(*Graphics::allocator, image, allocation);

	uint32_t old_mip_levels = mip_levels;
	VkImageCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.imageType = props.is_1D ? VK_IMAGE_TYPE_1D : (props.depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D);
	ci.format = ImageFormatEnum::toVulkanType(props.format);
	ci.tiling = props.tiling;
	ci.usage = props.usage;
	ci.flags = props.is_cubemap * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	ci.initialLayout = props.initial_layout;
	ci.extent = VkExtent3D(props.width, props.height, props.depth);
	ci.arrayLayers = props.layers;
	mip_levels = props.mipmap * std::floor(std::log2(std::max(props.width, props.height))) + 1;
	ci.mipLevels = mip_levels;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.samples = props.samples;

	bool mip_levels_changed = old_mip_levels != mip_levels;

	VmaAllocationCreateInfo allocation_info = {};
	allocation_info.usage = props.memory_usage;
	Graphics::vulkanAssert(VkResult(vmaCreateImage(*Graphics::allocator, &ci, &allocation_info, &image, &allocation, nullptr)));

	if (props.create_view && (layers != props.layers || mip_levels_changed))
	{
		view = makeUnique<ImageView>(this->image, props.format, mip_levels, props.layers, (props.is_cubemap ? ImageViewType::CUBEMAP : (props.is_1D ? ImageViewType::IMAGE1D : (props.depth == 1 ? ImageViewType::IMAGE2D : ImageViewType::IMAGE3D))));
	}
	if (props.create_sampler && mip_levels_changed)
	{
		SamplerProps sampler_props = props.sampler_props;
		sampler_props.mip_levels = mip_levels;
		sampler_props.linear_mipmap = props.mipmap_filter != VK_FILTER_NEAREST;
		sampler = makeUnique<Sampler>(sampler_props);
	}
}

void Image::insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer)
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
	barrier.subresourceRange.baseArrayLayer = base_layer;
	barrier.subresourceRange.layerCount = layers;
	Graphics::getActiveCommandBuffer().pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), {}, {}, { barrier });
}

void Image::generateMipmaps()
{
	if (mip_levels <= 1)
		return;

	VkFormatProperties format_properties = Graphics::physical_device->getFormatProperties(ImageFormatEnum::toVulkanType(props.format));

	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT,
		"Image format does not support src blitting");
	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT,
		"Image format does not support dst blitting");

	CommandBuffer command_buffer{};
	command_buffer.begin();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = props.layers;
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
		Graphics::getActiveCommandBuffer().pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags(0), {}, {}, { barrier });
		
		VkImageBlit blit{};
		blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
		blit.srcOffsets[1] = VkOffset3D(mip_width, mip_height, 1);
		blit.srcSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = props.layers;
		blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
		blit.dstOffsets[1] = VkOffset3D(mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1);
		blit.dstSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = props.layers;

		Graphics::getActiveCommandBuffer().blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { blit }, props.mipmap_filter);

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = props.layout;
		layout = barrier.newLayout;
		Graphics::getActiveCommandBuffer().pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, {}, {}, { barrier });

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
	Graphics::getActiveCommandBuffer().pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, {}, {}, { barrier });

	command_buffer.submitIdle();
}

void Image::setData(void* data, uint32_t base_layer, uint32_t layers)
{
	VkImageSubresource image_subresource{};
	image_subresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
	image_subresource.mipLevel = 0;
	image_subresource.arrayLayer = base_layer;
	VkSubresourceLayout subresource_layout = Graphics::logical_device->getImageSubresourceLayout(image, image_subresource);

	if (needs_staging)
	{
		StagingBuffer sb(data, subresource_layout.size * layers);
		void* buffer_data;
		sb.map(&buffer_data);
		memcpy((uint8_t*)buffer_data + subresource_layout.offset, data, sb.getSize());
		sb.unmap();
		copyFromBuffer(sb);
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy((uint8_t*)buffer_data + subresource_layout.offset, data, subresource_layout.size * layers);
		unmap();
	}
}

void Image::getData(void* data, uint32_t base_layer, uint32_t layers)
{
	VkImageSubresource image_subresource{};
	image_subresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
	image_subresource.mipLevel = 0;
	image_subresource.arrayLayer = base_layer;
	VkSubresourceLayout subresource_layout = Graphics::logical_device->getImageSubresourceLayout(image, image_subresource);
	
	if (needs_staging)
	{
		StagingBuffer sb(nullptr, subresource_layout.size * layers);
		copyToBuffer(sb, base_layer, layers);
		void* buffer_data;
		sb.map(&buffer_data);
		std::memcpy(data, (const uint8_t*)buffer_data + subresource_layout.offset, subresource_layout.size * layers);
		sb.unmap();
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy(data, (const uint8_t*)buffer_data + subresource_layout.offset, subresource_layout.size * layers);
		unmap();
	}
}

bool Image::copyImage(const shared<Image>& destination, uint32_t layer)
{
	bool supports_blit = true;
	
	if(!isFeatureSupported(VK_FORMAT_FEATURE_BLIT_SRC_BIT))
	{
		SK_WARN("Device does not support blitting from optimal tiled images, using copy instead of blit!");
		supports_blit = false;
	}

	if (!destination->isFeatureSupported(VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		SK_WARN("Device does not support blitting to linear tiled images, using copy instead of blit!");
		supports_blit = false;
	}

	//Do the actual blit from the swapchain image to our host visible destination image.
	CommandBuffer command_buffer;
	command_buffer.begin();
	VkImageLayout last_destination_layout = destination->getLayout();
	destination->insertMemoryBarrier(VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);

	VkImageLayout last_layout = getLayout(); 
	insertMemoryBarrier(VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, layer);

	//If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
	if (supports_blit) 
	{
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = layer;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
		blit.srcOffsets[1] = VkOffset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
		blit.dstSubresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(destination->getProps().format);
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
		blit.dstOffsets[1] = VkOffset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
		Graphics::getActiveCommandBuffer().blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { blit }, VK_FILTER_NEAREST);
	}
	else 
	{
		//Otherwise use image copy (requires us to manually flip components).
		VkImageCopy region{};
		region.srcSubresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(props.format);
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = layer;
		region.srcSubresource.layerCount = 1;
		region.dstSubresource.aspectMask = ImageFormatEnum::getVulkanAspectFlags(destination->getProps().format);
		region.dstSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = 1;
		region.extent = VkExtent3D(props.width, props.height, props.depth);
		Graphics::getActiveCommandBuffer().copyImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { region });
	}

	destination->insertMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, last_destination_layout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);
	insertMemoryBarrier(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, last_layout, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, layer);

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