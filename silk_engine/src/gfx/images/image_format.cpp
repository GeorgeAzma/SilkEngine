#include "image_format.h"

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