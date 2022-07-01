#pragma once

#include "image.h"

struct Image1DProps : public ImageProps
{
private:
	using ImageProps::height;
	using ImageProps::depth;
	using ImageProps::layers;
};

class Image1D : public Image
{
public:
	Image1D(const Image1DProps& props);
	Image1D(uint32_t width, ImageFormat format = ImageFormat::BGRA);
};