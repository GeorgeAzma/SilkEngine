#include "image_array.h"
#include "image2D.h"

ImageArray::ImageArray(const std::vector<std::string>& files, const ImageArrayProps& props)
{
	this->props = props;
	this->props.layers = files.size();

	Bitmap data{};
	data.load(files);
	this->props.width = data.width;
	this->props.height = data.height;
	if (data.channels == 3)
		data.align4();
	this->props.data = data.data();
	this->props.format = ImageFormatEnum::fromChannelCount(data.channels);
	create(this->props);
}

ImageArray::ImageArray(const ImageArrayProps& props)
{
	this->props = props;
	create(this->props);
}