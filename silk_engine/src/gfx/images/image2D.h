#pragma once

#include "image.h"

class Image2D : public Image
{
public:
	Image2D(const Image2DProps& props);
	Image2D(std::string_view file, const Image2DProps& props = {});
	Image2D(VkImage image, const Image2DProps& props = {});

	std::string_view getPath() const { return path; }

	static Bitmap load(std::string_view file);

private:
	std::string path = "";
};