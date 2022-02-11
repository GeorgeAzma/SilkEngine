#include "cubemap.h"
#include "image2D.h"

Cubemap::Cubemap(const std::vector<std::string>& files, const CubemapProps& props)
{
	this->props = props;
	ImageData load_data = load(files);
	this->props.width = load_data.width;
	this->props.height = load_data.height;
	if (load_data.channels == 3)
		align4(load_data);
	this->props.data = load_data.data.data();
	this->props.format = getDefaultFormatFromChannelCount(load_data.channels);
	create(this->props);
}

Cubemap::Cubemap(const CubemapProps& props)
{
	this->props = props;
	create(this->props);
}

ImageData Cubemap::load(const std::vector<std::string>& files)
{
	SK_ASSERT(files.size() == 6, "You have to specify exactly 6 filepaths for loading cube array, instead of: {0}", files.size());

	std::vector<std::string> paths(files.size());
	for (size_t i = 0; i < files.size(); ++i)
		paths[i] = std::string("data/images/") + files[i];
	
	ImageData load_data;
	
	ImageData data = Image2D::load(paths[0]);
	if (data.channels == 3)
		Image::align4(data);
	
	load_data.width = data.width;
	load_data.height = data.height;
	load_data.channels = data.channels;
	
	size_t size = load_data.width * load_data.height * load_data.channels;
	
	load_data.data.resize(size * paths.size());
	
	std::memcpy(load_data.data.data(), data.data.data(), size * sizeof(uint8_t));
	
	for (size_t i = 1; i < paths.size(); ++i)
	{
		data = Image2D::load(paths[i]);
		if (data.channels == 3)
			Image::align4(data);
	
		SK_ASSERT(data.width == load_data.width
			&& data.height == load_data.height
			&& data.channels == load_data.channels,
			"Error while loading cubemap, couldn't load image at {0}. width, height and channel count should match in all the faces of the cubemap", paths[i]);
	
		std::memcpy(load_data.data.data() + i * size, data.data.data(), size * sizeof(uint8_t));
	}
	
	return load_data;
}
