#pragma once

#include "image.h"

struct ImageBarrierProps
{
	VkImage image = VK_NULL_HANDLE;
	VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout new_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	uint32_t mip_levels = 1;
	uint32_t base_mip_level = 0;
	uint32_t layers = 1;
	uint32_t base_layer = 0;
};

class ImageBarrier
{
public:
	ImageBarrier(const ImageBarrierProps& props);
};