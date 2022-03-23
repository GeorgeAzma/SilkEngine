#include "image_array.h"
#include "image2D.h"

ImageArray::ImageArray(const std::vector<std::string>& files, const ImageArrayProps& props)
	: paths(files.size())
{
	this->props = props;
	Bitmap load_data = load(files);
	this->props.width = load_data.width;
	this->props.height = load_data.height;
	this->props.array_layers = files.size();
	if (load_data.channels == 3)
		align4(load_data);
	this->props.data = load_data.data.data();
	this->props.format = getDefaultFormatFromChannelCount(load_data.channels);
	create(this->props);
}

ImageArray::ImageArray(const ImageArrayProps& props)
{
	this->props = props;
	create(this->props);
}

Bitmap ImageArray::load(const std::vector<std::string>& files)
{
	SK_ASSERT(files.size(), "You have to specify at least one filepath for loading image array");

	Bitmap load_data;
	Bitmap data = Image2D::load(files[0]);
	if (data.channels == 3)
		Image::align4(data);

	load_data.width = data.width;
	load_data.height = data.height;
	load_data.channels = data.channels;

	size_t size = load_data.width * load_data.height * load_data.channels;

	load_data.data.resize(size * files.size());

	std::memcpy(load_data.data.data(), data.data.data(), size * sizeof(uint8_t));

	for (size_t i = 1; i < files.size(); ++i)
	{
		data = Image2D::load(files[i]);
		if (data.channels == 3)
			Image::align4(data);

		SK_ASSERT(data.width == load_data.width
			&& data.height == load_data.height
			&& data.channels == load_data.channels,
			"Error while loading image array, couldn't load image at {0}. width, height and channel count should match in all the images of image array", files[i]);

		std::memcpy(load_data.data.data() + i * size, data.data.data(), size * sizeof(uint8_t));
	}

	return load_data;
}