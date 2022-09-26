#pragma once

#include "image.h"

class ImageView : NonCopyable
{
public:
	ImageView(VkImage image, Image::Format format, size_t base_mip_level = 0, uint32_t mip_levels = 1, size_t base_layer = 0, size_t layers = 1, Image::Type image_type = Image::Type::_2D, VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY });
	ImageView(const Image& image, size_t base_mip_level = 0, uint32_t mip_levels = 0, size_t base_layer = 0, size_t layers = 0, VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY });
	~ImageView();

	operator const VkImageView& () const { return image_view; }

private:
	VkImageView image_view = nullptr;
};