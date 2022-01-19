#include "image.h"
#include "graphics.h"
#include "buffers/buffer.h"
#include "enums.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Image::Image(const std::string& file)
{
	descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	load(file);
	create(props);
}

Image::Image(const ImageProps& props)
{
	descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create(props);
}

Image::~Image()
{
	staging_buffer = nullptr;
	view = nullptr;
	sampler = nullptr;
	vmaDestroyImage(*Graphics::allocator, image, allocation);
}

void Image::load(const std::string& file)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VE_ASSERT(pixels, "Failed to load image: {0}", file);
	props.width = width;
	props.height = height;
	props.format = getDefaultFormatFromChannelCount(channels);

	size_t size = width * height * channels;
	staging_buffer = makeUnique<StagingBuffer>(pixels, size);

	stbi_image_free(pixels);
}

void Image::create(const ImageProps& props)
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
	create_info.arrayLayers = 1;
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
		copyBufferToImage();
		if (props.mipmap)
		{
			generateMipmaps();
		}
	}

	transitionLayout(props.layout);
	
	if (props.create_view)
	{
		view = makeUnique<ImageView>(image, props.format, mip_levels);
		descriptor_image_info.imageView = *view;
	}
	if (props.create_sampler)
	{
		SamplerProps sampler_props{};
		sampler_props.mip_levels = mip_levels;
		sampler = makeUnique<Sampler>(sampler_props);
		descriptor_image_info.sampler = *sampler;
	}
}

void Image::transitionLayout(VkImageLayout newLayout)
{
	if (descriptor_image_info.imageLayout == newLayout)
		return;

	CommandBuffer command_buffer;
	command_buffer.begin();

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
	barrier.subresourceRange.layerCount = 1;

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

void Image::copyBufferToImage()
{
	CommandBuffer command_buffer;
	command_buffer.begin();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = EnumInfo::getAspectFlags(props.format);
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { props.width, props.height, 1 };

	vkCmdCopyBufferToImage(command_buffer, *staging_buffer, image, descriptor_image_info.imageLayout, 1, &region);

	command_buffer.end();
	command_buffer.submit();
	command_buffer.wait();
}

void Image::generateMipmaps()
{
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(*Graphics::physical_device, props.format, &format_properties);

	VE_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
		"Image format does not support linear blitting");

	CommandBuffer command_buffer;
	command_buffer.begin();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = EnumInfo::getAspectFlags(props.format);
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
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
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = 
		{ 
			mip_width > 1 ? mip_width / 2 : 1, 
			mip_height > 1 ? mip_height / 2 : 1, 1 
		};
		blit.dstSubresource.aspectMask = barrier.subresourceRange.aspectMask;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

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

Image::TransitionInfo Image::getTransitionInfo(VkImageLayout oldLayout, VkImageLayout newLayout)
{
	TransitionInfo transition_info{};

	//WARNING: This only supports two cases, add more in future
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
		VE_ERROR("unsupported layout transition: old layout - {0}, new layout - {1}", oldLayout, newLayout);
	}

	return transition_info;
}

VkFormat Image::getDefaultFormatFromChannelCount(int channels)
{
	switch (channels)
	{
	case 1: return VK_FORMAT_R8_SRGB;
	case 2: return VK_FORMAT_R8G8_SRGB;
	case 3: return VK_FORMAT_R8G8B8_SRGB;
	case 4: return VK_FORMAT_R8G8B8A8_SRGB;
	}

	VE_ERROR("Unsupported channel count specified: {0}", channels);
	return VkFormat(0);
}