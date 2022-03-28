#include "image2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Image2D::Image2D(const Image2DProps& props)
{
	this->props = props;
	create(this->props);
}

Image2D::Image2D(std::string_view file, const Image2DProps& props)
	: path(std::string("data/images/") + file.data())
{
	this->props = props;
	Bitmap load_data = load(file);
	this->props.width = load_data.width;
	this->props.height = load_data.height;
	if (load_data.channels == 3)
		align4(load_data);
	this->props.data = load_data.data.data();
	this->props.format = getDefaultFormatFromChannelCount(load_data.channels);
	create(this->props);
}

Image2D::Image2D(VkImage image, const Image2DProps& props)
{
	this->props = props;
	this->image = image;
	create(this->props);
}

Bitmap Image2D::load(std::string_view file)
{
	std::string path = std::string("data/images/") + file.data();

	Bitmap image_data{};
	stbi_uc* pixels = stbi_load(path.c_str(), &image_data.width, &image_data.height, &image_data.channels, 0);
	SK_ASSERT(pixels, "Failed to load image: {0}", path);

	image_data.data.resize(image_data.width * image_data.height * image_data.channels);
	std::memcpy(image_data.data.data(), pixels, image_data.data.size() * sizeof(uint8_t));

	stbi_image_free(pixels);

	SK_TRACE("Image Loaded: {0}", path);
	return image_data;
}