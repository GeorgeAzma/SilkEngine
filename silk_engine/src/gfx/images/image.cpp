#include "image.h"
#include "image_barrier.h"
#include "gfx/graphics.h"
#include "gfx/buffers/buffer.h"
#include "gfx/enums.h"
#include "gfx/buffers/command_buffer.h"
#include "gfx/devices/physical_device.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Image::Image(const std::string& file, const ImageProps& props)
	: path(std::string("data/images/") + file), props(props)
{
	ImageData load_data = load(file);
	this->props.width = load_data.width;
	this->props.height = load_data.height;
	if (load_data.channels == 3)
		align4(load_data);
	this->props.format = getDefaultFormatFromChannelCount(load_data.channels);
	staging_buffer = makeShared<StagingBuffer>(load_data.data.data(), load_data.data.size() * sizeof(uint8_t));
	create(this->props);
}

Image::Image(const ImageProps& props)
	: props(props)
{
	if (props.data)
		staging_buffer = makeShared<StagingBuffer>(props.data, props.width * props.height * EnumInfo::formatSize(props.format));
	create(props);
}

Image::Image(VkImage image, const ImageProps& props)
	: image(image), props(props)
{
	if (props.data)
		staging_buffer = makeShared<StagingBuffer>(props.data, props.width * props.height * EnumInfo::formatSize(props.format));
	create(props);
}

Image::~Image()
{
	staging_buffer = nullptr;
	view = nullptr;
	sampler = nullptr;
	if(allocation != VK_NULL_HANDLE)
		vmaDestroyImage(*Graphics::allocator, image, allocation);
}

ImageData Image::load(const std::string& file)
{
	std::string path = std::string("data/images/") + file;

	int width, height, channels;
	stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
	SK_ASSERT(pixels, "Failed to load image: {0}", path);

	std::vector<uint8_t> pixels_vec(width * height * channels);
	std::memcpy(pixels_vec.data(), pixels, pixels_vec.size());

	stbi_image_free(pixels);

	SK_TRACE("Image Loaded: {0}", path);
	return { width, height, channels, pixels_vec };
}

void Image::align4(ImageData& image)
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
	if (image == VK_NULL_HANDLE)
	{
		VkImageCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.imageType = VK_IMAGE_TYPE_2D;
		create_info.extent.width = props.width;
		create_info.extent.height = props.height;
		create_info.extent.depth = 1;
		create_info.arrayLayers = 1;
		create_info.format = props.format;
		create_info.tiling = props.tiling;
		create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		create_info.usage = props.usage;
		if (props.mipmap)
		{
			mip_levels = std::floor(std::log2(std::max(props.width, props.height))) + 1;
			mip_levels = std::max(std::min(mip_levels, Graphics::physical_device->getImageFormatProperties(create_info.format, create_info.imageType, create_info.tiling, create_info.usage, create_info.flags).maxMipLevels), 1u);
		}
		create_info.mipLevels = mip_levels;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.samples = props.samples;

		VmaAllocationCreateInfo allocation_info = {};
		allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		Graphics::vulkanAssert(vmaCreateImage(*Graphics::allocator, &create_info, &allocation_info, &image, &allocation, nullptr));
	}

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
		SamplerProps sampler_props = props.sampler_props;
		sampler_props.mip_levels = mip_levels;
		sampler_props.linear_mipmap = props.mipmap_filter != VK_FILTER_NEAREST; //This is a bit of automation, but is also harmful for control
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

	ImageBarrierProps barrier_props{};
	barrier_props.image = image;
	barrier_props.old_layout = descriptor_image_info.imageLayout;
	barrier_props.new_layout = new_layout;
	barrier_props.aspect = EnumInfo::getAspectFlags(props.format);
	barrier_props.mip_levels = mip_levels;
	ImageBarrier barrier(barrier_props);

	command_buffer.end();
	command_buffer.submitIdle();

	descriptor_image_info.imageLayout = new_layout;
}

void Image::copyBufferToImage()
{
	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

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
	command_buffer.submitIdle();
}

void Image::generateMipmaps()
{
	if (mip_levels <= 1)
		return;

	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(*Graphics::physical_device, props.format, &format_properties);

	SK_ASSERT(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
		"Image format does not support linear blitting");

	CommandBuffer command_buffer;
	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	int32_t mip_width = props.width;
	int32_t mip_height = props.height;

	ImageBarrierProps barrier_props{};
	barrier_props.image = image;
	barrier_props.old_layout = descriptor_image_info.imageLayout;
	barrier_props.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier_props.aspect = EnumInfo::getAspectFlags(props.format);
	barrier_props.mip_levels = mip_levels;
	ImageBarrier transfer_barrier(barrier_props);

	barrier_props.mip_levels = 1;

	for (uint32_t i = 1; i < mip_levels; ++i)
	{
		barrier_props.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier_props.new_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier_props.base_mip_level = i - 1;
		ImageBarrier barrier(barrier_props);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = barrier_props.aspect;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = 
		{ 
			mip_width > 1 ? mip_width / 2 : 1, 
			mip_height > 1 ? mip_height / 2 : 1, 1 
		};
		blit.dstSubresource.aspectMask = barrier_props.aspect;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(command_buffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			props.mipmap_filter);

		barrier_props.old_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier_props.new_layout = props.layout;
		descriptor_image_info.imageLayout = barrier_props.new_layout;
		ImageBarrier reset_barrier(barrier_props);

		if (mip_width > 1) 
			mip_width /= 2;

		if (mip_height > 1) 
			mip_height /= 2;
	}

	barrier_props.base_mip_level = mip_levels - 1;
	barrier_props.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier_props.new_layout = props.layout;
	ImageBarrier barrier(barrier_props);

	command_buffer.end();
	command_buffer.submitIdle();
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

	SK_ERROR("Unsupported channel count specified: {0}", channels);
	return VkFormat(0);
}