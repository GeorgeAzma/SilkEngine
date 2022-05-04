#include "image_format.h"

ImageFormat ImageFormatEnum::fromVulkanType(VkFormat vulkan_format)
{
	using enum ImageFormat;
	switch (vulkan_format)
	{
		case VK_FORMAT_R8_UNORM: return RED;
		case VK_FORMAT_R8G8_UNORM: return RG;
		case VK_FORMAT_R8G8B8_UNORM: return RGB;
		case VK_FORMAT_R8G8B8A8_UNORM: return RGBA;
		case VK_FORMAT_B8G8R8A8_UNORM: return BGRA;
		case VK_FORMAT_D24_UNORM_S8_UINT: return DEPTH_STENCIL;
		case VK_FORMAT_D32_SFLOAT: return DEPTH;
		case VK_FORMAT_S8_UINT: return STENCIL;
	}

	SK_ERROR("Unsupported vulkan format specified: {0}.", vulkan_format);
	return ImageFormat(0);
}

ImageFormat ImageFormatEnum::fromChannelCount(uint8_t channels)
{
	using enum ImageFormat;
	switch (channels)
	{
		case 1: return RED;
		case 2: return RG;
		case 3: return RGB;
		case 4: return RGBA;
	}

	SK_ERROR("Unsupported image format channel count specified: {0}.", channels);
}

VkFormat ImageFormatEnum::toVulkanType(ImageFormat format)
{
	using enum ImageFormat;
	switch (format)	
	{
		case RED: return VK_FORMAT_R8_UNORM;
		case RG: return VK_FORMAT_R8G8_UNORM;
		case RGB: return VK_FORMAT_R8G8B8_UNORM;
		case RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
		case BGRA: return VK_FORMAT_B8G8R8A8_UNORM;
		case DEPTH_STENCIL: return VK_FORMAT_D24_UNORM_S8_UINT;
		case DEPTH: return VK_FORMAT_D32_SFLOAT;
		case STENCIL: return VK_FORMAT_S8_UINT;
	}

	SK_ERROR("Unsupported image format specified: {0}.", format);
	return VkFormat(0);
}

size_t ImageFormatEnum::getSize(ImageFormat format)
{
	using enum ImageFormat;
	switch (format)
	{
		case RED: return 1;
		case RG: return 2;
		case RGB: return 3;
		case RGBA: return 4;
		case BGRA: return 4;
		case DEPTH_STENCIL: return 4;
		case DEPTH: return 4;
		case STENCIL: return 1;
	}

	SK_ERROR("Unsupported image format specified: {0}.", format);
	return 0;
}

VkImageAspectFlags ImageFormatEnum::getVulkanAspectFlags(ImageFormat format)
{
	if (hasDepth(format))
		return VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil(format) * VK_IMAGE_ASPECT_STENCIL_BIT);

	if (hasStencil(format))
		return VK_IMAGE_ASPECT_STENCIL_BIT;

	return VK_IMAGE_ASPECT_COLOR_BIT;
}

uint8_t ImageFormatEnum::getChannelCount(ImageFormat format)
{
	using enum ImageFormat;
	switch (format)
	{
		case RED: return 1;
		case RG: return 2;
		case RGB: return 3;
		case RGBA: return 4;
		case BGRA: return 4;
		case DEPTH_STENCIL: return 1; //NOTE: Might be 2, probably not gonna use this anyways tho
		case DEPTH: return 1;
		case STENCIL: return 1;
	}

	SK_ERROR("Unsupported image format specified: {0}.", format);
	return 0;
}

bool ImageFormatEnum::hasStencil(ImageFormat format)
{
	using enum ImageFormat;
	return format == STENCIL || format == DEPTH_STENCIL;
}

bool ImageFormatEnum::hasDepth(ImageFormat format)
{
	using enum ImageFormat;
	return format == DEPTH || format == DEPTH_STENCIL;
}