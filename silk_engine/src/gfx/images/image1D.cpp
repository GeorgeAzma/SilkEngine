#include "image1D.h"

Image1D::Image1D(const Image1DProps& props)
{
	this->props = props;
	create(this->props);
}

Image1D::Image1D(uint32_t width, ImageFormat format)
{
	Image1DProps props{};
	props.width = width;
	props.format = format;
	this->props = props;
	create(this->props);
}