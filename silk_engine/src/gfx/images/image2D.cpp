#include "image2D.h"

Image2D::Image2D(const Image2DProps& props)
{
	this->props = props;
	create(this->props);
}

Image2D::Image2D(uint32_t width, uint32_t height, ImageFormat format)
{
	Image2DProps props{};
	props.width = width;
	props.height = height;
	props.format = format;
	this->props = props;
	create(this->props);
}

Image2D::Image2D(std::string_view file, const Image2DProps& props)
{
	this->props = props;
	Bitmap data{};
	data.load(file);
	this->props.width = data.width;
	this->props.height = data.height;
	if (data.channels == 3)
		data.align4();
	this->props.data = data.data.data();
	this->props.format = ImageFormatEnum::fromChannelCount(data.channels);
	create(this->props);
}

Image2D::Image2D(VkImage image, const Image2DProps& props)
{
	this->props = props;
	this->image = image;
	create(this->props);
}