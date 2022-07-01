#include "cubemap_array.h"
#include "bitmap.h"

CubemapArray::CubemapArray(const std::vector<std::string>& files, const CubemapArrayProps& props)
{
	SK_ASSERT(files.size() % 6, "Size of cubemap array file paths should be devisable by 6 (for each cube face), but you specified: {0} files", files.size());
	this->props = props;
	this->props.layers = files.size() / 6;

	Bitmap data{};
	data.load(files);
	if (data.channels == 3)
		data.align4();
	this->props.width = data.width;
	this->props.height = data.height;
	this->props.format = ImageFormatEnum::fromChannelCount(data.channels);
	this->props.data = data.data();
	create(this->props);
}

CubemapArray::CubemapArray(const CubemapArrayProps& props)
{
	this->props = props;
	create(this->props);
}