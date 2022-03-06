#include "image.h"
#include "gfx/graphics.h"
#include "gfx/buffers/buffer.h"
#include "gfx/enums.h"
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

void Image::align4(Bitmap& image)
{
	//TODO: This can be done in compute shader
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
	unique<StagingBuffer> staging_buffer = props.data ? makeUnique<StagingBuffer>(props.data, getSize()) : nullptr;
	descriptor_image_info.imageLayout = props.initial_layout;

	if ((const VkImage&)image == VK_NULL_HANDLE)
	{
#ifdef SK_ENABLE_DEBUG_OUTPUT
		if (props.usage & vk::ImageUsageFlagBits::eStorage)
		{
			vk::FormatProperties format_properties = Graphics::physical_device->getFormatProperties(props.format);
			SK_ASSERT(((props.tiling == vk::ImageTiling::eOptimal) ? format_properties.optimalTilingFeatures : format_properties.linearTilingFeatures) & vk::FormatFeatureFlagBits::eStorageImage, "Storage image doesn't support specified format.");
		}
#endif
		vk::ImageCreateInfo ci{};
		ci.imageType = props.is_1D ? vk::ImageType::e1D : (props.depth == 1 ? vk::ImageType::e2D : vk::ImageType::e3D);
		ci.format = props.format;
		ci.tiling = props.tiling;
		ci.usage = props.usage;
		ci.flags = props.is_cubemap ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlagBits(0);
		ci.initialLayout = props.initial_layout;
		ci.extent = vk::Extent3D(props.width, props.height, props.depth);
		ci.arrayLayers = props.array_layers;
		if (props.mipmap)
		{
			mip_levels = std::floor(std::log2(std::max(props.width, props.height))) + 1;
			auto max_mip_levels = Graphics::physical_device->getImageFormatProperties(ci.format, ci.imageType, ci.tiling, ci.usage, ci.flags).maxMipLevels;
		}
		ci.mipLevels = mip_levels;
		ci.sharingMode = vk::SharingMode::eExclusive;
		ci.samples = props.samples;

		VmaAllocationCreateInfo allocation_info = {};
		allocation_info.usage = props.memory_usage;

		const VkImageCreateInfo& vk_ci = (const VkImageCreateInfo&)ci;
		VkImage& vk_image = (VkImage&)image;
		Graphics::vulkanAssert(vk::Result(vmaCreateImage(*Graphics::allocator, &vk_ci, &allocation_info, &vk_image, &allocation, nullptr)));
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
		view = makeUnique<ImageView>(this->image, props.format, mip_levels, props.array_layers, (props.is_cubemap ? ImageViewType::CUBEMAP : (props.is_1D ? ImageViewType::IMAGE1D : (props.depth == 1 ? ImageViewType::IMAGE2D : ImageViewType::IMAGE3D))));
		descriptor_image_info.imageView = *view;
	}
	if (props.create_sampler)
	{
		SamplerProps sampler_props = props.sampler_props;
		sampler_props.mip_levels = mip_levels;
		sampler_props.linear_mipmap = props.mipmap_filter != vk::Filter::eNearest;
		sampler = makeUnique<Sampler>(sampler_props);
		descriptor_image_info.sampler = *sampler;
	}
}

void Image::transitionLayout(vk::ImageLayout new_layout) 
{
	if (descriptor_image_info.imageLayout == new_layout || new_layout == vk::ImageLayout::eUndefined)
		return;

	CommandBuffer command_buffer;
	command_buffer.begin();

	vk::ImageMemoryBarrier barrier = {};
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
	barrier.srcAccessMask = vk::AccessFlags(0);
	barrier.dstAccessMask = vk::AccessFlags(0);

	// TODO: this pipeline barrier code was written by total noob, check if it's correct
	// Source access mask controls actions that have to be finished on the old layout before it will be transitioned to the new layout.
	vk::PipelineStageFlags source_stage = vk::PipelineStageFlagBits::eAllCommands;
	vk::PipelineStageFlags destination_stage = vk::PipelineStageFlagBits::eAllCommands;
	switch (barrier.oldLayout)
	{
		case vk::ImageLayout::eUndefined:
			source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		case vk::ImageLayout::eGeneral:
			source_stage = vk::PipelineStageFlagBits::eHost;
			break;
		case vk::ImageLayout::ePreinitialized:
			barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
			source_stage = vk::PipelineStageFlagBits::eHost;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			source_stage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eTransferSrcOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			source_stage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eColorAttachmentOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			source_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			source_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
			source_stage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		default:
			SK_ERROR("Unsupported image layout transition source: {0}", barrier.oldLayout);
			break;
	}

	// Destination access mask controls the dependency for the new image layout.
	switch (new_layout)
	{
		case vk::ImageLayout::eUndefined:
			destination_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		case vk::ImageLayout::eGeneral:
			destination_stage = vk::PipelineStageFlagBits::eHost;
			break;
		case vk::ImageLayout::ePreinitialized:
			barrier.dstAccessMask = vk::AccessFlagBits::eHostWrite;
			destination_stage = vk::PipelineStageFlagBits::eHost;
			break;
		case vk::ImageLayout::eTransferDstOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			destination_stage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eTransferSrcOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			destination_stage = vk::PipelineStageFlagBits::eTransfer;
			break;
		case vk::ImageLayout::eColorAttachmentOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			destination_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			destination_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests; 
			break;
		case vk::ImageLayout::eShaderReadOnlyOptimal:
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		default:
			SK_ERROR("Unsupported image layout transition destination: {0}", barrier.oldLayout);
			break;
	}
	Graphics::active.command_buffer.pipelineBarrier(source_stage, destination_stage, vk::DependencyFlags(0), {}, {}, { barrier });

	command_buffer.submitIdle();

	descriptor_image_info.imageLayout = new_layout;
}

void Image::insertMemoryBarrier(vk::AccessFlags source_access_mask, vk::AccessFlags destination_access_mask, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::PipelineStageFlags source_stage_mask, vk::PipelineStageFlags destination_stage_mask, uint32_t base_mip_level, uint32_t base_array_layer)
{
	if (descriptor_image_info.imageLayout == new_layout || new_layout == vk::ImageLayout::eUndefined)
		return;
	insertMemoryBarrier(image, source_access_mask, destination_access_mask, old_layout, new_layout, source_stage_mask, destination_stage_mask, getAspectFlags(), mip_levels, base_mip_level, props.array_layers, base_array_layer);
	descriptor_image_info.imageLayout = new_layout;
}

bool Image::isFeatureSupported(vk::FormatFeatureFlags feature) const
{
	vk::FormatProperties format_properties = Graphics::physical_device->getFormatProperties(props.format);
	const vk::FormatFeatureFlags& features = (props.tiling == vk::ImageTiling::eOptimal) ? format_properties.optimalTilingFeatures : format_properties.linearTilingFeatures;
	return bool(features & feature);
}

void Image::copyFromBuffer(vk::Buffer buffer)
{
	transitionLayout(vk::ImageLayout::eTransferDstOptimal);

	CommandBuffer command_buffer;
	command_buffer.begin();

	size_t offset = 0;
	std::vector<vk::BufferImageCopy> regions(props.array_layers);
	for (size_t layer = 0; layer < props.array_layers; ++layer)
	{
		vk::BufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = getAspectFlags();
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = layer;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(props.width, props.height, props.depth);

		offset += props.width * props.height * props.depth * formatSize(props.format);

		regions[layer] = std::move(region);
	}
	Graphics::active.command_buffer.copyBufferToImage(buffer, image, descriptor_image_info.imageLayout, regions);

	command_buffer.submitIdle();
}

void Image::copyToBuffer(vk::Buffer buffer, uint32_t base_array_layer, uint32_t array_layers)
{
	transitionLayout(vk::ImageLayout::eTransferSrcOptimal);

	CommandBuffer command_buffer;
	command_buffer.begin();

	vk::BufferImageCopy region{};
	region.imageOffset = vk::Offset3D(0, 0, 0);
	region.imageExtent = vk::Extent3D(props.width, props.height, props.depth);
	region.bufferOffset = 0;
	region.bufferImageHeight = props.height;
	region.bufferRowLength = props.width;
	region.imageSubresource.aspectMask = getAspectFlags();
	region.imageSubresource.baseArrayLayer = base_array_layer;
	region.imageSubresource.layerCount = array_layers;
	region.imageSubresource.mipLevel = 0;
	Graphics::active.command_buffer.copyImageToBuffer(image, descriptor_image_info.imageLayout, buffer, { region });

	command_buffer.submitIdle();
}

void Image::insertMemoryBarrier(const vk::Image& image, vk::AccessFlags source_access_mask, vk::AccessFlags destination_access_mask, vk::ImageLayout old_layout, vk::ImageLayout new_layout, vk::PipelineStageFlags source_stage_mask, vk::PipelineStageFlags destination_stage_mask, vk::ImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t array_layers, uint32_t base_array_layer)
{
	vk::ImageMemoryBarrier barrier{};
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
	Graphics::active.command_buffer.pipelineBarrier(source_stage_mask, destination_stage_mask, vk::DependencyFlags(0), {}, {}, { barrier });
}

void Image::generateMipmaps()
{
	if (mip_levels <= 1)
		return;

	vk::FormatProperties format_properties = Graphics::physical_device->getFormatProperties(props.format);

	SK_ASSERT(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc,
		"Image format does not support src blitting");
	SK_ASSERT(format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst,
		"Image format does not support dst blitting");

	CommandBuffer command_buffer{};
	command_buffer.begin();

	vk::ImageMemoryBarrier barrier{};
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
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.subresourceRange.baseMipLevel = i - 1;
		Graphics::active.command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(0), {}, {}, { barrier });
		
		vk::ImageBlit blit{};
		blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.srcOffsets[1] = vk::Offset3D(mip_width, mip_height, 1);
		blit.srcSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = props.array_layers;
		blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.dstOffsets[1] = vk::Offset3D(mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1);
		blit.dstSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = props.array_layers;

		Graphics::active.command_buffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, { blit }, props.mipmap_filter);

		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = props.layout;
		descriptor_image_info.imageLayout = barrier.newLayout;
		Graphics::active.command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(0), {}, {}, { barrier });

		if (mip_width > 1) 
			mip_width /= 2;

		if (mip_height > 1) 
			mip_height /= 2;
	}

	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = props.layout;
	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	Graphics::active.command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(0), {}, {}, { barrier });

	command_buffer.submitIdle();
}

void Image::setData(void* data, uint32_t base_array_layer, uint32_t array_layers)
{
	vk::ImageSubresource image_subresource = {};
	image_subresource.aspectMask = getAspectFlags();
	image_subresource.mipLevel = 0;
	image_subresource.arrayLayer = base_array_layer;
	vk::SubresourceLayout subresource_layout = Graphics::logical_device->getImageSubresourceLayout(image, image_subresource);

	if (needs_staging)
	{
		StagingBuffer sb(data, subresource_layout.size * array_layers);
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
		std::memcpy((uint8_t*)buffer_data + subresource_layout.offset, data, subresource_layout.size * array_layers);
		unmap();
	}
}

void Image::getData(void* data, uint32_t base_array_layer, uint32_t array_layers)
{
	vk::ImageSubresource image_subresource = {};
	image_subresource.aspectMask = getAspectFlags();
	image_subresource.mipLevel = 0;
	image_subresource.arrayLayer = base_array_layer;
	vk::SubresourceLayout subresource_layout = Graphics::logical_device->getImageSubresourceLayout(image, image_subresource);
	
	if (needs_staging)
	{
		StagingBuffer sb(nullptr, subresource_layout.size * array_layers);
		copyToBuffer(sb, base_array_layer, array_layers);
		void* buffer_data;
		sb.map(&buffer_data);
		std::memcpy(data, (const uint8_t*)buffer_data + subresource_layout.offset, subresource_layout.size * array_layers);
		sb.unmap();
	}
	else
	{
		void* buffer_data;
		map(&buffer_data);
		std::memcpy(data, (const uint8_t*)buffer_data + subresource_layout.offset, subresource_layout.size * array_layers);
		unmap();
	}
}

bool Image::copyImage(shared<Image> destination, uint32_t array_layer)
{
	bool supports_blit = true;
	
	if(!isFeatureSupported(vk::FormatFeatureFlagBits::eBlitSrc))
	{
		SK_WARN("Device does not support blitting from optimal tiled images, using copy instead of blit!");
		supports_blit = false;
	}

	if (!destination->isFeatureSupported(vk::FormatFeatureFlagBits::eBlitDst))
	{
		SK_WARN("Device does not support blitting to linear tiled images, using copy instead of blit!\n");
		supports_blit = false;
	}

	//Do the actual blit from the swapchain image to our host visible destination image.
	CommandBuffer command_buffer;
	command_buffer.begin();
	auto destination_old_layout = destination->getDescriptorInfo().imageLayout;
	destination->insertMemoryBarrier(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite, destination_old_layout, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, 0, 0);
	auto old_layout = descriptor_image_info.imageLayout;
	insertMemoryBarrier(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eTransferRead, old_layout, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, 0, array_layer);

	//If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
	if (supports_blit) 
	{
		vk::ImageBlit blit = {};
		blit.srcSubresource.aspectMask = getAspectFlags();
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = array_layer;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.srcOffsets[1] = vk::Offset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
		blit.dstSubresource.aspectMask = destination->getAspectFlags();
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[0] = vk::Offset3D(0, 0, 0);
		blit.dstOffsets[1] = vk::Offset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
		Graphics::active.command_buffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, *destination, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eNearest);
	}
	else 
	{
		//Otherwise use image copy (requires us to manually flip components).
		vk::ImageCopy region{};
		region.srcSubresource.aspectMask = getAspectFlags();
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = array_layer;
		region.srcSubresource.layerCount = 1;
		region.dstSubresource.aspectMask = destination->getAspectFlags();
		region.dstSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = 1;
		region.extent = vk::Extent3D(props.width, props.height, props.depth);
		Graphics::active.command_buffer.copyImage(image, vk::ImageLayout::eTransferSrcOptimal, *destination, vk::ImageLayout::eTransferDstOptimal, { region });
	}

	destination->insertMemoryBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eTransferDstOptimal, destination_old_layout, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, 0, 0);
	insertMemoryBarrier(vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eTransferSrcOptimal, old_layout, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, 0, array_layer);

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

vk::Format Image::getDefaultFormatFromChannelCount(int channels)
{
	using enum vk::Format;
	switch (channels)
	{
	case 0: return vk::Format(0);
	case 1: return eR8Unorm;
	case 2: return eR8G8Unorm;
	case 3: return eR8G8B8Unorm;
	case 4: return eR8G8B8A8Unorm;
	}

	SK_ERROR("Unsupported channel count specified: {0}", channels);
	return vk::Format(0);
}

bool Image::hasStencil(vk::Format format)
{
	using enum vk::Format;
	return format == eD32SfloatS8Uint
		|| format == eD24UnormS8Uint
		|| format == eD16UnormS8Uint
		|| format == eS8Uint;
}

bool Image::hasDepth(vk::Format format)
{
	using enum vk::Format;
	return format == eD32Sfloat 
		|| format == eD32SfloatS8Uint
		|| format == eD24UnormS8Uint
		|| format == eD16UnormS8Uint;
}

vk::ImageAspectFlags Image::getAspectFlags(vk::Format format)
{
	using enum vk::ImageAspectFlagBits;
	if (!hasDepth(format))
		return eColor;

	if (hasStencil(format))
		return eDepth | eStencil;

	return eDepth;
}

size_t Image::channelCount(vk::Format format)
{
	using enum vk::Format;
	switch (format)
	{
	case eR8Sint:				return 1;
	case eR8Uint:				return 1;
	case eR16Sint:				return 1;
	case eR16Uint:				return 1;
	case eR32Sint:				return 1;
	case eR32Uint:				return 1;
	case eR32Sfloat:			return 1;
	case eR64Sfloat:			return 1;
	case eR32G32Sfloat:			return 2;
	case eR32G32B32Sfloat:		return 3;
	case eR32G32B32A32Sfloat:	return 4;
	case eR32G32Sint:			return 2;
	case eR32G32B32Sint:		return 3;
	case eR32G32B32A32Sint:		return 4;
	case eR32G32Uint:			return 2;
	case eR32G32B32Uint:		return 3;
	case eR32G32B32A32Uint:		return 4;
	case eR64G64Sfloat:			return 2;
	case eR64G64B64Sfloat:		return 3;
	case eR64G64B64A64Sfloat:	return 4;
	case eR8Srgb:				return 1;
	case eR8G8Srgb:				return 2;
	case eR8G8B8Srgb:			return 3;
	case eR8G8B8A8Srgb:			return 4;
	case eR8Unorm:				return 1;
	case eR8G8Unorm:			return 2;
	case eR8G8B8Unorm:			return 3;
	case eR8G8B8A8Unorm:		return 4;
	case eD16Unorm:				return 1;
	case eD16UnormS8Uint:		return 1;
	case eD24UnormS8Uint:		return 1;
	case eD32Sfloat:			return 1;
	case eD32SfloatS8Uint:		return 1;
	case eB8G8R8Srgb:			return 3;
	case eB8G8R8A8Srgb:			return 4;
	case eB8G8R8Unorm:			return 3;
	case eB8G8R8A8Unorm:		return 4;
	}

	SK_ERROR("Unsupported format specified: {0}.", format);
	return 0;
}

Type Image::formatToType(vk::Format format)
{
	using enum vk::Format;
	switch (format)
	{
	case eR8Sint:				return Type::BYTE;
	case eR8Uint:				return Type::UBYTE;
	case eR16Sint:				return Type::SHORT;
	case eR16Uint:				return Type::USHORT;
	case eR32Sint:				return Type::INT;
	case eR32Uint:				return Type::UINT;
	case eR32Sfloat:			return Type::FLOAT;
	case eR64Sfloat:			return Type::DOUBLE;
	case eR32G32Sfloat:			return Type::VEC2;
	case eR32G32B32Sfloat:		return Type::VEC3;
	case eR32G32B32A32Sfloat:	return Type::VEC4;
	case eR32G32Sint:			return Type::VEC2I;
	case eR32G32B32Sint:		return Type::VEC3I;
	case eR32G32B32A32Sint:		return Type::VEC4I;
	case eR32G32Uint:			return Type::VEC2U;
	case eR32G32B32Uint:		return Type::VEC3U;
	case eR32G32B32A32Uint:		return Type::VEC4U;
	case eR64G64Sfloat:			return Type::VEC2D;
	case eR64G64B64Sfloat:		return Type::VEC3D;
	case eR64G64B64A64Sfloat:	return Type::VEC4D;
	case eR8Srgb:				return Type::FLOAT;
	case eR8G8Srgb:				return Type::VEC2;
	case eR8G8B8Srgb:			return Type::VEC3;
	case eR8G8B8A8Srgb:			return Type::VEC4;
	case eR8Unorm:				return Type::FLOAT;
	case eR8G8Unorm:			return Type::VEC2;
	case eR8G8B8Unorm:			return Type::VEC3;
	case eR8G8B8A8Unorm:		return Type::VEC4;
	case eD16Unorm:				return Type::UINT;
	case eD16UnormS8Uint:		return Type::UINT;
	case eD24UnormS8Uint:		return Type::UINT;
	case eD32Sfloat:			return Type::FLOAT;
	case eD32SfloatS8Uint:		return Type::DOUBLE;
	case eB8G8R8Srgb:			return Type::VEC3;
	case eB8G8R8A8Srgb:			return Type::VEC4;
	case eB8G8R8Unorm:			return Type::VEC3;
	case eB8G8R8A8Unorm:		return Type::VEC4;
	}

	SK_ERROR("Unsupported format specified: {0}.", format);
	return Type(0);
}

size_t Image::formatSize(vk::Format format)
{
	using enum vk::Format;
	switch (format)
	{
	case eR8Sint:				return 1;
	case eR8Uint:				return 1;
	case eR16Sint:				return 2;
	case eR16Uint:				return 2;
	case eR32Sint:				return 4;
	case eR32Uint:				return 4;
	case eR32Sfloat:			return 4;
	case eR64Sfloat:			return 8;
	case eR32G32Sfloat:			return 8;
	case eR32G32B32Sfloat:		return 12;
	case eR32G32B32A32Sfloat:	return 16;
	case eR32G32Sint:			return 8;
	case eR32G32B32Sint:		return 12;
	case eR32G32B32A32Sint:		return 16;
	case eR32G32Uint:			return 8;
	case eR32G32B32Uint:		return 12;
	case eR32G32B32A32Uint:		return 16;
	case eR64G64Sfloat:			return 16;
	case eR64G64B64Sfloat:		return 24;
	case eR64G64B64A64Sfloat:	return 32;
	case eR8Srgb:				return 1;
	case eR8G8Srgb:				return 2;
	case eR8G8B8Srgb:			return 3;
	case eR8G8B8A8Srgb:			return 4;
	case eR8Unorm:				return 1;
	case eR8G8Unorm:			return 2;
	case eR8G8B8Unorm:			return 3;
	case eR8G8B8A8Unorm:		return 4;
	case eD16Unorm:				return 2;
	case eD16UnormS8Uint:		return 3;
	case eD24UnormS8Uint:		return 4;
	case eD32Sfloat:			return 8;
	case eD32SfloatS8Uint:		return 4;
	case eB8G8R8Srgb:			return 3;
	case eB8G8R8A8Srgb:			return 4;
	case eB8G8R8Unorm:			return 3;
	case eB8G8R8A8Unorm:		return 4;
	}

	SK_ERROR("Unsupported format specified: {0}.", format);
	return 0;
}