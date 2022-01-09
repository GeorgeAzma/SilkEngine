#include "image.h"
#include "graphics.h"
#include "buffers/buffer.h"

Image::Image(const std::string& file)
{
	load(file);

	ImageProps props{};
	props.width = width;
	props.height = height;
	props.format = getDefaultFormatFromChannelCount(channels);
	create(props);
}

Image::Image(const ImageProps& props)
{
	create(props);
}

Image::~Image()
{
	
}

void Image::load(const std::string& file)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	VE_CORE_ASSERT(pixels, "Failed to load image: {0}", file);
	this->width = width;
	this->height = height;
	this->channels = channels;

	size_t size = width * height * channels;
	staging_buffer = std::make_unique<StagingBuffer>(pixels, size);

	stbi_image_free(pixels);
}

void Image::create(const ImageProps& props)
{
	this->width = props.width;
	this->height = props.height;
	this->channels = EnumInfo::count(EnumInfo::formatToType(props.format));
	VkImageCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.imageType = VK_IMAGE_TYPE_2D;
	create_info.extent.width = width;
	create_info.extent.height = height;
	create_info.extent.depth = 1;
	create_info.mipLevels = 1;
	create_info.arrayLayers = 1;
	create_info.format = getDefaultFormatFromChannelCount(channels);
	create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	create_info.usage = props.usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.samples = props.samples;

	Graphics::vulkanAssert(vkCreateImage(*Graphics::logical_device, &create_info, nullptr, &image));

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(*Graphics::logical_device, image, &memory_requirements);

	VkMemoryAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocation_info.allocationSize = memory_requirements.size;
	allocation_info.memoryTypeIndex = Buffer::findMemoryType(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	Graphics::vulkanAssert(vkAllocateMemory(*Graphics::logical_device, &allocation_info, nullptr, &memory));

	vkBindImageMemory(*Graphics::logical_device, image, memory, 0);

	transitionLayout(create_info.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage();
	transitionLayout(create_info.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Image::transitionLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	CommandBuffer command_buffer;
	command_buffer.begin();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0; //TODO
	barrier.dstAccessMask = 0; //TODO

	vkCmdPipelineBarrier(command_buffer, 0 /* TODO */, 0 /* TODO */, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	command_buffer.end();
}

void Image::copyBufferToImage()
{
	CommandBuffer command_buffer;
	command_buffer.begin();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(command_buffer, *staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	command_buffer.end();
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

	VE_CORE_ERROR("Unsupported channel count specified: {0}", channels);
	return VkFormat(0);
}