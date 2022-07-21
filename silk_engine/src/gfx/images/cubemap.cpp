#include "cubemap.h"
#include "raw_image.h"

Cubemap::Cubemap(const std::vector<std::string>& files, const CubemapProps& props)
{
	SK_ASSERT(files.size() == 6, "You have to specify exactly 6 filepaths for loading cubemap, instead you specified: {0}", files.size());
	this->props = props;
	RawImage data{};
	data.load(files);
	this->props.width = data.width;
	this->props.height = data.height;
	if (data.channels == 3)
		data.align4();
	this->props.data = data.pixels.data();
	this->props.format = ImageFormatEnum::fromChannelCount(data.channels);
	create(this->props);
}

Cubemap::Cubemap(const CubemapProps& props)
{
	this->props = props;
	create(this->props);
}