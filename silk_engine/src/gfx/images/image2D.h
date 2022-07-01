#pragma once

#include "image.h"

struct Image2DProps : public ImageProps
{
public:
	Image2DProps() = default;

private:
	using ImageProps::depth;
	using ImageProps::layers;
};

class Image2D : public Image
{
public:
	Image2D(const Image2DProps& props);
	Image2D(uint32_t width, uint32_t height, ImageFormat format = ImageFormat::BGRA);
	Image2D(std::string_view file, const Image2DProps& props = {});
	// Constructor used for swap chain image creation ONLY
	Image2D(VkImage image, ImageFormat format);
};