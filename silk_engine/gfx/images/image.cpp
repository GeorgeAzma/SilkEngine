#include "image.h"
#include "raw_image.h"
#include "gfx/window/window.h"
#include "gfx/render_context.h"
#include "gfx/buffers/buffer.h"
#include "gfx/devices/physical_device.h"
#include "gfx/devices/logical_device.h"
#include "gfx/window/swap_chain.h"
#include "gfx/buffers/command_buffer.h"
#include "image_view.h"
#include "gfx/allocators/allocator.h"

VkImageType Image::getVulkanTypeFromType(Type type)
{
	switch (type)
	{
	case Type::_1D:
	case Type::_1D_ARRAY:
		return VK_IMAGE_TYPE_1D;
	case Type::_2D:
	case Type::_2D_ARRAY:
	case Type::CUBE:
	case Type::CUBE_ARRAY:
		return VK_IMAGE_TYPE_2D;
	case Type::_3D:
		return VK_IMAGE_TYPE_3D;
	}

	return VK_IMAGE_TYPE_2D;
}

Image::Format Image::getFormatFromChannelCount(uint8_t channels)
{
	switch (channels)
	{
	case 1: return Format::RED;
	case 2: return Format::RG;
	case 3: return Format::RGB;
	case 4: return Format::RGBA;
	}

	return Format::BGRA;
}

size_t Image::getFormatSize(Format format)
{
	switch (format)
	{
	case Format::RED: return 1;
	case Format::RG: return 2;
	case Format::RGB: return 3;
	case Format::RGBA: return 4;
	case Format::BGRA: return 4;
	case Format::DEPTH16: return 2;
	case Format::DEPTH: return 4;
	case Format::STENCIL: return 1;
	case Format::DEPTH16_STENCIL: return 3;
	case Format::DEPTH24_STENCIL: return 4;
	case Format::DEPTH_STENCIL: return 5;
	}

	return 4;
}

VkImageAspectFlags Image::getFormatVulkanAspectFlags(Format format)
{
	if (isDepthFormat(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT | (isStencilFormat(format) * VK_IMAGE_ASPECT_STENCIL_BIT);

	if (isStencilFormat(format))
		return VK_IMAGE_ASPECT_STENCIL_BIT;

	return VK_IMAGE_ASPECT_COLOR_BIT;
}

uint8_t Image::getFormatChannelCount(Format format)
{
	switch (format)
	{
	case Format::RED: return 1;
	case Format::RG: return 2;
	case Format::RGB: return 3;
	case Format::RGBA: return 4;
	case Format::BGRA: return 4;
	case Format::DEPTH16:
	case Format::DEPTH:
	case Format::STENCIL:
	case Format::DEPTH16_STENCIL:
	case Format::DEPTH24_STENCIL:
	case Format::DEPTH_STENCIL:  //NOTE: Might be 2, probably not gonna use this anyways tho
		return 1;
	}

	return 4;
}

uint32_t Image::calculateMipLevels(uint32_t width, uint32_t height, uint32_t depth)
{
	return floor(std::log2(std::max({ width, height, depth }))) + 1;
}

Image::Image(const Props& props)
	: props(props)
{
	create();
}

Image::Image(uint32_t width, Format format)
{
	props.width = width;
	props.format = format;
	props.type = Type::_1D;
	create();
}

Image::Image(uint32_t width, uint32_t height, Format format)
{
	props.width = width;
	props.height = height;
	props.format = format;
	create();
}

Image::Image(const path& file, const Props& p)
	: props(p)
{
	RawImage<> raw(file);
	if (raw.channels == 3)
		raw.align4();
	props.width = raw.width;
	props.height = raw.height;
	props.format = getFormatFromChannelCount(raw.channels);
	create();
	setData(raw.pixels.data());
	generateMipmaps();
}

Image::Image(uint32_t width, uint32_t height, Format format, VkImage img)
{
	props.width = width;
	props.height = height;
	props.format = format;
	props.sampler_props.mipmap_mode = Sampler::MipmapMode::NONE;
	props.allocation_props.priority = 1.0f;
	image = img;
	create();
}

Image::Image(const std::array<path, 6>& files, const Props& p)
	: props(p)
{
	RawImage<> raw(files);
	if (raw.channels == 3)
		raw.align4();
	props.width = raw.width;
	props.height = raw.height;
	props.format = getFormatFromChannelCount(raw.channels);
	create();
	setData(raw.pixels.data());
	generateMipmaps();
}

Image::Image(std::span<const path> files, const Props& p)
	: props(p)
{
	RawImage<> raw(files);
	if (raw.channels == 3)
		raw.align4();
	props.width = raw.width;
	props.height = raw.height;
	props.layers = files.size();
	props.format = getFormatFromChannelCount(raw.channels);
	create();
	setData(raw.pixels.data());
	generateMipmaps();
}

Image::Image(std::span<const std::array<path, 6>> files, const Props& p)
	: props(p)
{
	std::vector<path> continous_files(files.size() * 6);
	std::copy(files.front().begin(), files.back().end(), continous_files.begin());
	RawImage<> raw(continous_files);
	if (raw.channels == 3)
		raw.align4();
	props.width = raw.width;
	props.height = raw.height;
	props.layers = files.size();
	props.format = getFormatFromChannelCount(raw.channels);
	create();
	setData(raw.pixels.data());
	generateMipmaps();
}

Image::~Image()
{
	view = nullptr;
	sampler = nullptr;
	if (VmaAllocation(allocation) != nullptr)
		RenderContext::getAllocator().destroyImage(image, allocation);
}

VkDescriptorImageInfo Image::getDescriptorInfo() const
{
	VkDescriptorImageInfo descriptor_image_info{};
	descriptor_image_info.imageLayout = layout;
	descriptor_image_info.imageView = view ? *view : nullptr;
	descriptor_image_info.sampler = sampler ? *sampler : nullptr;
	return descriptor_image_info;
}

const VkImageView& Image::getView() const { return *view; }

void Image::transitionLayout(VkImageLayout new_layout)
{
	if (layout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;
	insertMemoryBarrier(image, layout, new_layout, getFormatVulkanAspectFlags(props.format), mip_levels, 0, props.layers, 0);
	RenderContext::execute();
	layout = new_layout;
}

bool Image::isFeatureSupported(VkFormatFeatureFlags feature) const
{
	VkFormatProperties format_properties = RenderContext::getPhysicalDevice().getFormatProperties(VkFormat(props.format));
	const VkFormatFeatureFlags& features = (props.tiling == VK_IMAGE_TILING_OPTIMAL) ? format_properties.optimalTilingFeatures : format_properties.linearTilingFeatures;
	return (features & feature) == feature;
}

void Image::copyFromBuffer(VkBuffer buffer, uint32_t base_layer, uint32_t layers)
{
	VkImageLayout old_layout = layout;
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	RenderContext::submit(
		[&](CommandBuffer& cb)
		{
			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = getFormatVulkanAspectFlags(props.format);
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = base_layer;
			region.imageSubresource.layerCount = layers;
			region.imageOffset = VkOffset3D(0, 0, 0);
			region.imageExtent = VkExtent3D(props.width, props.height, props.depth);
			cb.copyBufferToImage(buffer, image, layout, { region });
		});
	RenderContext::execute();

	transitionLayout(old_layout);
}

void Image::copyToBuffer(VkBuffer buffer, uint32_t base_layer, uint32_t layers)
{
	VkImageLayout old_layout = layout;
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	RenderContext::submit(
		[&](CommandBuffer& cb)
		{
			VkBufferImageCopy region{};
			region.imageOffset = VkOffset3D(0, 0, 0);
			region.imageExtent = VkExtent3D(props.width, props.height, props.depth);
			region.bufferOffset = 0;
			region.bufferImageHeight = props.height;
			region.bufferRowLength = props.width;
			region.imageSubresource.aspectMask = getFormatVulkanAspectFlags(props.format);
			region.imageSubresource.baseArrayLayer = base_layer;
			region.imageSubresource.layerCount = layers;
			region.imageSubresource.mipLevel = 0;
			cb.copyImageToBuffer(image, layout, buffer, { region });
		});
	RenderContext::execute();

	transitionLayout(old_layout);
}

void Image::insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, VkPipelineStageFlags source_stage_mask, VkPipelineStageFlags destination_stage_mask, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer)
{
	if (old_layout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;
	RenderContext::submit(
		[&](CommandBuffer& cb)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = source_access_mask;	// Source access mask controls actions that have to be finished on the old layout before it will be transitioned to the new layout.
			barrier.dstAccessMask = destination_access_mask; // Destination access mask controls the dependency for the new image layout.
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
			cb.pipelineBarrier(source_stage_mask, destination_stage_mask, VkDependencyFlags(0), {}, {}, { barrier });
		});
}

void Image::insertMemoryBarrier(const VkImage& image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer)
{
	VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkAccessFlags src_access = VK_ACCESS_NONE;
	switch (old_layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_GENERAL:
		src_stage = VK_PIPELINE_STAGE_HOST_BIT;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		src_access = VK_ACCESS_HOST_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_HOST_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		src_access = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		src_access = VK_ACCESS_TRANSFER_READ_BIT;
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		src_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		src_access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		src_access = VK_ACCESS_SHADER_READ_BIT;
		src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}

	VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkAccessFlags dst_access = VK_ACCESS_NONE;
	switch (new_layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		dst_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_GENERAL:
		dst_stage = VK_PIPELINE_STAGE_HOST_BIT;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		dst_access = VK_ACCESS_HOST_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_HOST_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		dst_access = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		dst_access = VK_ACCESS_TRANSFER_READ_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		dst_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		dst_access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (src_access == VK_ACCESS_NONE)
		{
			src_access = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			src_stage = VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		dst_access = VK_ACCESS_SHADER_READ_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}
	insertMemoryBarrier(image, src_access, dst_access, old_layout, new_layout, src_stage, dst_stage, aspect, mip_levels, base_mip_level, layers, base_layer);
}

void Image::create()
{
	if (image == nullptr)
	{
		VkImageCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ci.imageType = getVulkanTypeFromType(props.type);
		ci.format = VkFormat(props.format);
		ci.tiling = props.tiling;
		ci.usage = props.usage;
		ci.flags = (props.type == Type::CUBE) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		ci.initialLayout = props.initial_layout;
		layout = props.initial_layout;
		ci.extent = { props.width, props.height, props.depth };
		ci.arrayLayers = props.layers;
		mip_levels = (props.sampler_props.mipmap_mode != Sampler::MipmapMode::NONE) ? calculateMipLevels(props.width, props.height, props.depth) : 1;
		ci.mipLevels = mip_levels;
		ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.samples = props.samples;

		VmaAllocationCreateInfo alloc_ci{};
		alloc_ci.usage = (VmaMemoryUsage)props.allocation_props.preferred_device;
		alloc_ci.flags = props.allocation_props.flags;
		alloc_ci.priority = props.allocation_props.priority;
		allocation = RenderContext::getAllocator().allocateImage(ci, alloc_ci, image);

		if (props.usage & VK_IMAGE_USAGE_SAMPLED_BIT)
			sampler = Sampler::get(props.sampler_props);
	}

	if (props.create_view)
		view = makeUnique<ImageView>(*this);
}

void Image::generateMipmaps()
{
	if (mip_levels <= 1 || props.sampler_props.mipmap_mode == Sampler::MipmapMode::NONE)
		return;

	RenderContext::submit(
		[&](CommandBuffer& cb)
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = image;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = getFormatVulkanAspectFlags(props.format);
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = props.layers;
			barrier.subresourceRange.levelCount = 1;

			int32_t mip_width = props.width;
			int32_t mip_height = props.height;
			int32_t mip_depth = props.depth;

			for (uint32_t i = 1; i < mip_levels; ++i)
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.subresourceRange.baseMipLevel = i - 1;
				cb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VkDependencyFlags(0), {}, {}, { barrier });

				VkImageBlit blit{};
				blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
				blit.srcOffsets[1] = VkOffset3D(mip_width, mip_height, mip_depth);
				blit.srcSubresource.aspectMask = barrier.subresourceRange.aspectMask;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = props.layers;
				blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
				if (mip_width > 1) mip_width >>= 1;
				if (mip_height > 1) mip_height >>= 1;
				if (mip_depth > 1) mip_depth >>= 1;
				blit.dstOffsets[1] = VkOffset3D(mip_width, mip_height, mip_depth);
				blit.dstSubresource.aspectMask = barrier.subresourceRange.aspectMask;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = props.layers;

				cb.blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { blit }, VK_FILTER_LINEAR);

				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				layout = barrier.newLayout;
				cb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, {}, {}, { barrier });

			}

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.subresourceRange.baseMipLevel = mip_levels - 1;
			cb.pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, {}, {}, { barrier });

		});
	RenderContext::execute();
}

void Image::setData(const void* data, uint32_t base_layer, uint32_t layers)
{
	if (!data)
		return;

	if (allocation.isHostVisible())
	{
		uint32_t row_pitch = props.width * getFormatSize(props.format);
		uint32_t layer_size = row_pitch * props.height * props.depth;
		uint32_t aligned_row_pitch = math::align(row_pitch, (uint32_t)allocation.getAlignment());
		uint32_t aligned_size = aligned_row_pitch * props.width;

		if (aligned_row_pitch == row_pitch)
		{
			for (uint32_t i = 0; i < layers; ++i)
				allocation.setData((uint8_t*)data + layer_size * i, aligned_size, aligned_size * (base_layer + i));
		}
		else
		{
			uint8_t* aligned_data = new uint8_t[aligned_size];
			for (uint32_t i = 0; i < layers; ++i)
			{
				for (uint32_t j = 0; j < props.height; ++j)
					memcpy(aligned_data + aligned_row_pitch * j, (const uint8_t*)data + row_pitch * j + layer_size * i, row_pitch);
				allocation.setData((uint8_t*)aligned_data, aligned_size, aligned_size * (base_layer + i));
			}
			delete[] aligned_data;
		}
	}
	else
	{
		Buffer sb(props.width * props.height * props.depth * getFormatSize(props.format) * layers, Buffer::TRANSFER_SRC, { Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED });
		sb.setData(data);
		copyFromBuffer(sb, base_layer, layers);
	}
}

void Image::getData(void* data, uint32_t base_layer, uint32_t layers)
{
	size_t layer_size = props.width * props.height * props.depth * getFormatSize(props.format);
	if (allocation.isHostVisible())
	{
		void* buffer_data;
		allocation.getData(data, layer_size * layers, layer_size * base_layer);
	}
	else
	{
		Buffer sb(layer_size * layers, Buffer::TRANSFER_DST, { Allocation::RANDOM_ACCESS | Allocation::MAPPED });
		copyToBuffer(sb, base_layer, layers);
		sb.getData(data);
	}
}

bool Image::copyImage(Image& destination)
{
	bool supports_blit = true;

	if (!isFeatureSupported(VK_FORMAT_FEATURE_BLIT_SRC_BIT))
	{
		SK_WARN("Device does not support blitting from optimal tiled images, using copy instead of blit!");
		supports_blit = false;
	}

	if (!destination.isFeatureSupported(VK_FORMAT_FEATURE_BLIT_DST_BIT))
	{
		SK_WARN("Device does not support blitting to linear tiled images, using copy instead of blit!");
		supports_blit = false;
	}

	//Do the actual blit from the swapchain image to our host visible destination image.
	RenderContext::submit(
		[&](CommandBuffer& cb)
		{
			VkImageLayout last_destination_layout = destination.getLayout();
			destination.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkImageLayout last_layout = getLayout();
			transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			//If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
			if (supports_blit)
			{
				VkImageBlit blit{};
				blit.srcSubresource.aspectMask = getFormatVulkanAspectFlags(props.format);
				blit.srcSubresource.mipLevel = 0;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
				blit.srcOffsets[1] = VkOffset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
				blit.dstSubresource.aspectMask = getFormatVulkanAspectFlags(destination.getProps().format);
				blit.dstSubresource.mipLevel = 0;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;
				blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
				blit.dstOffsets[1] = VkOffset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
				cb.blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { blit }, VK_FILTER_NEAREST);
			}
			else
			{
				//Otherwise use image copy (requires us to manually flip components).
				VkImageCopy region{};
				region.srcSubresource.aspectMask = getFormatVulkanAspectFlags(props.format);
				region.srcSubresource.mipLevel = 0;
				region.srcSubresource.baseArrayLayer = 0;
				region.srcSubresource.layerCount = 1;
				region.dstSubresource.aspectMask = getFormatVulkanAspectFlags(destination.getProps().format);
				region.dstSubresource.mipLevel = 0;
				region.dstSubresource.baseArrayLayer = 0;
				region.dstSubresource.layerCount = 1;
				region.extent = VkExtent3D(props.width, props.height, props.depth);
				cb.copyImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { region });
			}

			destination.transitionLayout(last_destination_layout);
			transitionLayout(last_layout);
		});

	return supports_blit;
}