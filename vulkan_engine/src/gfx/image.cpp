#include "image.h"
#include "buffers/staging_buffer.h"

Image::Image()
{
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

	VkDeviceSize image_size = width * height * channels;
	StagingBuffer staging_buffer(pixels, image_size);
	stbi_image_free(pixels);

	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = getDefaultFormatFromChannelCount(channels);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

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
}
