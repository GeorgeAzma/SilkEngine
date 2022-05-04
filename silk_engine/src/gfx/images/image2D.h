#pragma once

#include "image.h"

class Image2D : public Image
{
public:
	Image2D(const Image2DProps& props);
	Image2D(uint32_t width, uint32_t height, VkFormat format = VK_FORMAT_B8G8R8A8_UNORM);
	Image2D(std::string_view file, const Image2DProps& props = {});
	Image2D(VkImage image, const Image2DProps& props = {});
};