#pragma once

#include "image.h"

struct ImageArrayProps : public ImageProps
{
private:
	using ImageProps::depth;
};

class ImageArray : public Image
{
public:
	ImageArray(const std::vector<std::string>& files, const ImageArrayProps& props = {});
	ImageArray(const ImageArrayProps& props);

	uint32_t getArrayLayers() const { return props.layers; }
};