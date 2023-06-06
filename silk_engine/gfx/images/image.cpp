#include "image.h"
#include "raw_image.h"
#include "image_view.h"
#include "silk_engine/gfx/window/window.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/buffers/buffer.h"
#include "silk_engine/gfx/devices/physical_device.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/window/swap_chain.h"
#include "silk_engine/gfx/buffers/command_buffer.h"
#include "silk_engine/gfx/allocators/allocator.h"

Image::Image(const Props& props)
	: props(props)
{
	create();
}

Image::Image(const RawImage<uint8_t>& raw_image, const Props& props)
	: props(props)
{
	this->props.width = raw_image.width;
	this->props.height = raw_image.height;
	this->props.format = getChannelsFormat(raw_image.channels);
	create();
	setData(raw_image.pixels.data());
	generateMipmaps();
}

Image::Image(uint32_t width, Format format)
{
	props.width = width;
	props.format = format;
	props.view_type = ImageViewType::_1D;
	create();
}

Image::Image(uint32_t width, uint32_t height, Format format)
{
	props.width = width;
	props.height = height;
	props.format = format;
	create();
}

Image::Image(const fs::path& file, const Props& props)
	: Image(RawImage(file), props)
{}

Image::Image(uint32_t width, uint32_t height, Format format, VkImage img)
{
	props.width = width;
	props.height = height;
	props.format = format;
	props.allocation_props.priority = 1.0f;
	image = img;
	create();
}

Image::Image(const std::array<fs::path, 6>& files, const Props& props)
	: props(props)
{
	RawImage raw_image(files);
	this->props.width = raw_image.width;
	this->props.height = raw_image.height;
	this->props.format = getChannelsFormat(raw_image.channels);
	this->props.view_type = ImageViewType::CUBE;
	create();
	setData(raw_image.pixels.data());
	generateMipmaps();
}

Image::Image(std::span<const fs::path> files, const Props& props)
	: props(props)
{
	RawImage raw_image(files);
	this->props.width = raw_image.width;
	this->props.height = raw_image.height;
	this->props.layers = files.size();
	this->props.format = getChannelsFormat(raw_image.channels);
	this->props.view_type = ImageViewType::_2D_ARRAY;
	create();
	setData(raw_image.pixels.data(), 0, 0);
	generateMipmaps();
}

Image::Image(std::span<const std::array<fs::path, 6>> files, const Props& props)
	: props(props)
{
	std::vector<fs::path> continous_files(files.size() * 6);
	for (size_t i = 0; i < files.size(); ++i)
		continous_files.insert(continous_files.begin() + i * 6, files[i].begin(), files[i].end());

	RawImage raw_image(continous_files);
	this->props.width = raw_image.width;
	this->props.height = raw_image.height;
	this->props.layers = files.size();
	this->props.format = getChannelsFormat(raw_image.channels);
	this->props.view_type = ImageViewType::CUBE_ARRAY;
	create();
	setData(raw_image.pixels.data(), 0, 0);
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
	descriptor_image_info.imageLayout = getLayout();
	descriptor_image_info.imageView = view ? *view : nullptr;
	descriptor_image_info.sampler = sampler ? *sampler : nullptr;
	return descriptor_image_info;
}

const VkImageView& Image::getView() const { return *view; }

void Image::setData(const void* data, uint32_t base_layer, uint32_t layers)
{
	if (!data)
		return;

	if (!layers) layers = props.layers - base_layer;
	if (VmaAllocation(allocation) && allocation.isHostVisible())
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
		Buffer sb(getSize(), BufferUsage::TRANSFER_SRC, { Allocation::SEQUENTIAL_WRITE | Allocation::MAPPED });
		sb.setData(data);
		copyFromBuffer(sb, base_layer, layers);
		RenderContext::execute();
	}
}

void Image::getData(void* data, uint32_t base_layer, uint32_t layers)
{
	size_t layer_size = getSize() / layers;
	if (VmaAllocation(allocation) && allocation.isHostVisible())
	{
		void* buffer_data;
		allocation.getData(data, layer_size * layers, layer_size * base_layer);
	}
	else
	{
		Buffer sb(layer_size * layers, BufferUsage::TRANSFER_DST, { Allocation::RANDOM_ACCESS | Allocation::MAPPED });
		copyToBuffer(sb, base_layer, layers);
		RenderContext::execute();
		sb.getData(data);
	}
}

bool Image::copyToImage(Image& destination)
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

	destination.transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB).
	if (supports_blit)
	{
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = ecast(getAspect());
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
		blit.srcOffsets[1] = VkOffset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
		blit.dstSubresource.aspectMask = ecast(destination.getAspect());
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
		blit.dstOffsets[1] = VkOffset3D((int32_t)props.width, (int32_t)props.height, (int32_t)props.depth);
		RenderContext::getCommandBuffer().blitImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { blit }, VK_FILTER_NEAREST);
	}
	else
	{
		// Otherwise use image copy (requires us to manually flip components).
		VkImageCopy region{};
		region.srcSubresource.aspectMask = ecast(getAspect());
		region.srcSubresource.mipLevel = 0;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = 1;
		region.dstSubresource.aspectMask = ecast(destination.getAspect());
		region.dstSubresource.mipLevel = 0;
		region.dstSubresource.baseArrayLayer = 0;
		region.dstSubresource.layerCount = 1;
		region.extent = VkExtent3D(props.width, props.height, props.depth);
		RenderContext::getCommandBuffer().copyImage(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { region });
	}

	return supports_blit;
}

void Image::copyFromBuffer(VkBuffer buffer, uint32_t base_layer, uint32_t layers)
{
	if (!layers) layers = props.layers - base_layer;
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 1, 0, layers, base_layer);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = ecast(getAspect());
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = base_layer;
	region.imageSubresource.layerCount = layers;
	region.imageOffset = VkOffset3D(0, 0, 0);
	region.imageExtent = VkExtent3D(props.width, props.height, props.depth);
	RenderContext::getCommandBuffer().copyBufferToImage(buffer, image, getLayout(region.imageSubresource.mipLevel, region.imageSubresource.baseArrayLayer), { region });
}

void Image::copyToBuffer(VkBuffer buffer, uint32_t base_layer, uint32_t layers)
{
	if (!layers) layers = props.layers - base_layer;
	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1, 0, layers, base_layer);
	VkBufferImageCopy region{};
	region.imageOffset = VkOffset3D(0, 0, 0);
	region.imageExtent = VkExtent3D(props.width, props.height, props.depth);
	region.bufferOffset = 0;
	region.bufferImageHeight = props.height;
	region.bufferRowLength = props.width;
	region.imageSubresource.aspectMask = ecast(getAspect());
	region.imageSubresource.baseArrayLayer = base_layer;
	region.imageSubresource.layerCount = layers;
	region.imageSubresource.mipLevel = 0;
	RenderContext::getCommandBuffer().copyImageToBuffer(image, getLayout(region.imageSubresource.mipLevel, region.imageSubresource.baseArrayLayer), buffer, { region });
}

void Image::transitionLayout(VkImageLayout new_layout, VkDependencyFlags dependency, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer)
{
	if (new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;
	if (!mip_levels) mip_levels = this->mip_levels - base_mip_level;
	if (!layers) layers = props.layers - base_layer;

	for (uint32_t layer = 0; layer < layers; ++layer)
		for (uint32_t mip = 0; mip < mip_levels; ++mip)
		{
			if (getLayout(mip + base_mip_level, layer + base_layer) == new_layout)
				continue;
			insertMemoryBarrier(image, getLayout(mip + base_mip_level, layer + base_layer), new_layout, dependency, getAspect(), 1, mip + base_mip_level, 1, layer + base_layer);
			setLayout(new_layout, 1, mip + base_mip_level, 1, layer + base_layer);
		}
}

bool Image::isFeatureSupported(VkFormatFeatureFlags feature) const
{
	VkFormatProperties format_properties = RenderContext::getPhysicalDevice().getFormatProperties(VkFormat(props.format));
	const VkFormatFeatureFlags& features = (props.linear_tiling) ? format_properties.linearTilingFeatures : format_properties.optimalTilingFeatures;
	return (features & feature) == feature;
}

void Image::create()
{
	layouts.resize(1, VK_IMAGE_LAYOUT_UNDEFINED);
	if (image == nullptr)
	{
		VkImageCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ci.imageType = VkImageType(getImageViewTypeImageType(props.view_type));
		ci.format = VkFormat(props.format);
		ci.tiling = props.linear_tiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		ci.usage = ecast(props.usage);
		ci.flags = (props.view_type == ImageViewType::CUBE) * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		ci.initialLayout = props.initial_layout;
		ci.extent = { props.width, props.height, props.depth };
		ci.arrayLayers = props.layers;
		// Multisampled images are never mip mapped
		mip_levels = (props.samples != VK_SAMPLE_COUNT_1_BIT || props.sampler_props.mipmap_mode == MipmapMode::NONE) ? 1 : calculateMipLevels(props.width, props.height, props.depth);
		if (mip_levels > 1)
			ci.usage |= ecast(ImageUsage::TRANSFER_DST | ImageUsage::TRANSFER_SRC);
		ci.mipLevels = mip_levels;
		ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.samples = props.samples;
		layouts.resize(mip_levels * props.layers, VK_IMAGE_LAYOUT_UNDEFINED);
		setLayout(props.initial_layout);

		VmaAllocationCreateInfo alloc_ci{};
		alloc_ci.usage = (VmaMemoryUsage)props.allocation_props.preferred_device;
		alloc_ci.flags = props.allocation_props.flags;
		alloc_ci.priority = props.allocation_props.priority;
		allocation = RenderContext::getAllocator().allocateImage(ci, alloc_ci, image);

		if (bool(props.usage & ImageUsage::SAMPLED))
			sampler = Sampler::get(props.sampler_props);
	}

	// Determining if image should create ImageView based on usage (Note: this is probably correct, but not sure)
	if (bool(props.usage & (ImageUsage::SAMPLED | ImageUsage::STORAGE | ImageUsage::COLOR_ATTACHMENT | ImageUsage::DEPTH_STENCIL_ATTACHMENT | ImageUsage::INPUT_ATTACHMENT | ImageUsage::TRANSIENT_ATTACHMENT)))
		view = makeShared<ImageView>(*this);
}

void Image::generateMipmaps()
{
	if (mip_levels <= 1 || props.sampler_props.mipmap_mode == MipmapMode::NONE)
		return;

	int32_t mip_width = props.width;
	int32_t mip_height = props.height;
	int32_t mip_depth = props.depth;

	transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	for (uint32_t i = 1; i < mip_levels; ++i)
	{
		transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1, i - 1);

		VkImageBlit blit{};
		blit.srcOffsets[0] = VkOffset3D(0, 0, 0);
		blit.srcOffsets[1] = VkOffset3D(mip_width, mip_height, mip_depth);
		blit.srcSubresource.aspectMask = ecast(getAspect());
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = props.layers;

		if (mip_width > 1) mip_width >>= 1;
		if (mip_height > 1) mip_height >>= 1;
		if (mip_depth > 1) mip_depth >>= 1;

		blit.dstOffsets[0] = VkOffset3D(0, 0, 0);
		blit.dstOffsets[1] = VkOffset3D(mip_width, mip_height, mip_depth);
		blit.dstSubresource.aspectMask = ecast(getAspect());
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = props.layers;

		RenderContext::getCommandBuffer().blitImage(image, getLayout(i - 1), image, getLayout(i), { blit }, VK_FILTER_LINEAR);
	}
	RenderContext::execute();
}

void Image::insertMemoryBarrier(const VkImage& image, VkAccessFlags source_access_mask, VkAccessFlags destination_access_mask, VkImageLayout old_layout, VkImageLayout new_layout, PipelineStage source_stage, PipelineStage destination_stage, VkDependencyFlags dependency, ImageAspect aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer)
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
	barrier.subresourceRange.aspectMask = ecast(aspect);
	barrier.subresourceRange.baseMipLevel = base_mip_level;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = base_layer;
	barrier.subresourceRange.layerCount = layers;
	RenderContext::getCommandBuffer().pipelineBarrier(source_stage, destination_stage, dependency, {}, {}, { barrier });
}

void Image::insertMemoryBarrier(const VkImage& image, VkImageLayout old_layout, VkImageLayout new_layout, VkDependencyFlags dependency, ImageAspect aspect, uint32_t mip_levels, uint32_t base_mip_level, uint32_t layers, uint32_t base_layer)
{
	if (old_layout == new_layout || new_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;
	PipelineStage src_stage = PipelineStage::ALL_COMMANDS;
	VkAccessFlags src_access = VK_ACCESS_NONE;
	switch (old_layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		src_stage = PipelineStage::TOP;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: // NOTE: probably doesn't work in some cases 
		src_stage = PipelineStage::TRANSFER;
		break;
	case VK_IMAGE_LAYOUT_GENERAL:
		src_stage = PipelineStage::HOST;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		src_access = VK_ACCESS_HOST_WRITE_BIT;
		src_stage = PipelineStage::HOST;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		src_access = VK_ACCESS_TRANSFER_WRITE_BIT;
		src_stage = PipelineStage::TRANSFER;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		src_access = VK_ACCESS_TRANSFER_READ_BIT;
		src_stage = PipelineStage::TRANSFER;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		src_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src_stage = PipelineStage::COLOR_ATTACHMENT_OUTPUT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		src_access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src_stage = PipelineStage::EARLY_FRAGMENT_TESTS | PipelineStage::LATE_FRAGMENT_TESTS;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		src_access = VK_ACCESS_SHADER_READ_BIT;
		src_stage = PipelineStage::FRAGMENT;
		break;
	}

	PipelineStage dst_stage = PipelineStage::ALL_COMMANDS;
	VkAccessFlags dst_access = VK_ACCESS_NONE;
	switch (new_layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		dst_stage = PipelineStage::TOP;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: // NOTE: probably doesn't work in some cases 
		dst_stage = PipelineStage::TRANSFER;
		break;
	case VK_IMAGE_LAYOUT_GENERAL:
		dst_stage = PipelineStage::HOST;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		dst_access = VK_ACCESS_HOST_WRITE_BIT;
		dst_stage = PipelineStage::HOST;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		dst_access = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst_stage = PipelineStage::TRANSFER;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		dst_access = VK_ACCESS_TRANSFER_READ_BIT;
		dst_stage = PipelineStage::TRANSFER;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		dst_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dst_stage = PipelineStage::COLOR_ATTACHMENT_OUTPUT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		dst_access |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dst_stage = PipelineStage::EARLY_FRAGMENT_TESTS | PipelineStage::LATE_FRAGMENT_TESTS;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (src_access == VK_ACCESS_NONE)
		{
			src_access = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			src_stage = PipelineStage::HOST | PipelineStage::TRANSFER;
		}
		dst_access = VK_ACCESS_SHADER_READ_BIT;
		dst_stage = PipelineStage::FRAGMENT;
		break;
	}
	insertMemoryBarrier(image, src_access, dst_access, old_layout, new_layout, src_stage, dst_stage, dependency, aspect, mip_levels, base_mip_level, layers, base_layer);
}

shared<Image> Image::add(std::string_view name, const shared<Image>& image)
{
	images.insert_or_assign(name, image);
	RenderContext::getLogicalDevice().setObjectName(VK_OBJECT_TYPE_IMAGE, VkImage(*image), name.data());
	return image;
}