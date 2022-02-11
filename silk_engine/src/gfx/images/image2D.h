#pragma once

#include "image.h"

class Image2D : public Image
{
public:
	Image2D(const Image2DProps& props);
	Image2D(const std::string& file, const Image2DProps& props = {});
	Image2D(VkImage image, const Image2DProps& props = {});

	const std::string& getPath() const { return path; }

	static ImageData load(const std::string& file);

private:
	std::string path = "";
};