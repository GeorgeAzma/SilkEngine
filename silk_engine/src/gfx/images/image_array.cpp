#include "image_array.h"
#include "image.h"
#include "gfx/graphics.h"
#include "gfx/buffers/buffer.h"
#include "gfx/enums.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"
#include <stb_image.h>

ImageArray::ImageArray(const std::vector<std::string>& files, const ImageProps& props)
{
	std::vector<std::string> paths(files.size());
	for(size_t i = 0; i < files.size(); ++i)
		paths[i] = std::string("data/images/") + files[i];
	descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	auto load_data = load(paths);
	this->props.width = load_data.width;
	this->props.height = load_data.height;
	this->props.format = Image::getDefaultFormatFromChannelCount(load_data.channels);
	staging_buffer = load_data.staging_buffer;
	layer_count = paths.size();
	create(props);
}

ImageArray::ImageArray(const ImageProps& props, size_t layer_count)
	: layer_count(layer_count)
{
	descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (props.data)
	{
		size_t data_size = props.width * props.height * EnumInfo::formatSize(props.format) * layer_count;
		staging_buffer = makeShared<StagingBuffer>(props.data, data_size);
	}
	create(props);
}

ImageArray::~ImageArray()
{
	staging_buffer = nullptr;
	view = nullptr;
	sampler = nullptr;
	vmaDestroyImage(*Graphics::allocator, image, allocation);
}

ImageLoadData ImageArray::load(const std::vector<std::string>& files)
{
	int width, height, channels;
	std::vector<stbi_uc> file_pixels;
	for (size_t i = 0; i < files.size(); ++i)
	{
		stbi_uc* pixels = stbi_load(files[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
		SK_ASSERT(pixels, "Failed to load image: {0}", files[i]);
		size_t size = width * height * channels;
		if (file_pixels.empty())
			file_pixels.resize(size * files.size());
		memcpy(file_pixels.data() + size * i, pixels, size * sizeof(stbi_uc));
		stbi_image_free(pixels);
	}

	shared<StagingBuffer> staging_buffer = makeShared<StagingBuffer>(file_pixels.data(), file_pixels.size() * sizeof(stbi_uc));

	return { width, height, channels, staging_buffer };
}

void ImageArray::create(const ImageProps& props)
{
	this->props = props;

	VkImageCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent.width = props.width;
	create_info.extent.height = props.height;
	create_info.extent.depth = 1;
	if (props.mipmap)
	{
		mip_levels = std::floor(std::log2(std::max(props.width, props.height))) + 1;
	}
	create_info.mipLevels = mip_levels;
	create_info.arrayLayers = layer_count;
	create_info.format = props.format;
	create_info.tiling = props.tiling;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = props.usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.samples = props.samples;

	VmaAllocationCreateInfo allocation_info = {};
	allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	Graphics::vulkanAssert(vmaCreateImage(*Graphics::allocator, &create_info, &allocation_info, &image, &allocation, nullptr));

	if (staging_buffer.get() != nullptr)
	{
		transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImageArray();
		if (props.mipmap)
		{
			generateMipmaps();
		}
	}

	transitionLayout(props.layout);
	
	if (props.create_view)
	{
		view = makeUnique<ImageView>(image, props.format, mip_levels, layer_count);
		descriptor_image_info.imageView = *view;
	}
	if (props.create_sampler)
	{
		SamplerProps sampler_props = props.sampler_props;
		sampler_props.mip_levels = mip_levels;
		sampler_props.linear_mipmap = props.mipmap_filter != VK_FILTER_NEAREST; //This is a bit of automation, but is also harmful for control
		sampler = makeUnique<Sampler>(sampler_props);
		descriptor_image_info.sampler = *sampler;
	}
}

void ImageArray::transitionLayout(VkImageLayout newLayout)
{
	if (descriptor_image_info.imageLayout == newLayout)
		return;

	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = descriptor_image_info.imageLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = EnumInfo::getAspectFlags(props.format);
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layer_count;

	TransitionInfo transition_info = getTransitionInfo(barrier.oldLayout, barrier.newLayout);
	barrier.srcAccessMask = transition_info.source_access_mask;
	barrier.dstAccessMask = transition_info.destination_access_mask;

	vkCmdPipelineBarrier(command_buffer, transition_info.source_stage,
		transition_info.destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	command_buffer.end();
	command_buffer.submit();
	command_buffer.wait();

	descriptor_image_info.imageLayout = newLayout;
}

void ImageArray::copyBufferToImageArray()
{
	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	size_t offset = 0;
	std::vector<VkBufferImageCopy> copy_regions(layer_count);
	for (size_t layer = 0; layer < layer_count; ++layer)
	{
		VkBufferImageCopy copy_region{};
		copy_region.bufferOffset = offset;
		copy_region.bufferRowLength = 0;
		copy_region.bufferImageHeight = 0;
		copy_region.imageSubresource.aspectMask = EnumInfo::getAspectFlags(props.format);
		copy_region.imageSubresource.mipLevel = 0;
		copy_region.imageSubresource.baseArrayLayer = layer;
		copy_region.imageSubresource.layerCount = 1;
		copy_region.imageOffset = { 0, 0, 0 };
		copy_region.imageExtent = { props.width, props.height, 1 };

		offset += props.width * props.height * EnumInfo::formatSize(props.format);

		copy_regions[layer] = std::move(copy_region);
	}
	vkCmdCopyBufferToImage(command_buffer, *staging_buffer, image, descriptor_image_info.imageLayout, copy_regions.size(), copy_regions.data());
	
	command_buffer.end();
	command_buffer.submit();
	command_buffer.wait();
}

void ImageArray::generateMipmaps()
{
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(*Graphics::physical_device, props.format, &format_properties);

	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
		"ImageArray format does not support linear blitting");

	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = EnumInfo::getAspectFlags(props.format);
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layer_count;
	barrier.subresourceRange.levelCount = 1;

	const TransitionInfo transition_info = getTransitionInfo(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, props.layout);

	int32_t mip_width = props.width;
	int32_t mip_height = props.height;

	for (uint32_t i = 1; i < mip_levels; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = layer_count;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = 
		{ 
			mip_width > 1 ? mip_width / 2 : 1, 
			mip_height > 1 ? mip_height / 2 : 1, 1 
		};
		blit.dstSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = layer_count;

		vkCmdBlitImage(command_buffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			props.mipmap_filter);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = props.layout;
		descriptor_image_info.imageLayout = barrier.newLayout;
		barrier.srcAccessMask = transition_info.source_access_mask;
		barrier.dstAccessMask = transition_info.destination_access_mask;

		vkCmdPipelineBarrier(command_buffer,
			transition_info.source_stage, 
			transition_info.destination_stage, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mip_width > 1) 
			mip_width /= 2;

		if (mip_height > 1) 
			mip_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = props.layout;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	const TransitionInfo transition_info_dst = getTransitionInfo(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, props.layout);
	vkCmdPipelineBarrier(command_buffer,
		transition_info_dst.source_stage, transition_info_dst.destination_stage, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	command_buffer.end();
	command_buffer.submit();
	command_buffer.wait();
}

ImageArray::TransitionInfo ImageArray::getTransitionInfo(VkImageLayout oldLayout, VkImageLayout newLayout)
{
	TransitionInfo transition_info{};

	//TODO: This only supports few cases, add more in future
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		transition_info.source_access_mask = 0;
		transition_info.destination_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;

		transition_info.source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		transition_info.destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		transition_info.source_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
		transition_info.destination_access_mask = VK_ACCESS_SHADER_READ_BIT;

		transition_info.source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		transition_info.destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		transition_info.source_access_mask = 0;
		transition_info.destination_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		transition_info.source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		transition_info.destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		transition_info.source_access_mask = VK_ACCESS_TRANSFER_READ_BIT;
		transition_info.destination_access_mask = VK_ACCESS_SHADER_READ_BIT;

		transition_info.source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		transition_info.destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		SK_ERROR("unsupported layout transition: old layout - {0}, new layout - {1}", oldLayout, newLayout);
	}

	return transition_info;
}