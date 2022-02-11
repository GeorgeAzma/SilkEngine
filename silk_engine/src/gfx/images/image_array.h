#pragma once

#include "image.h"

class ImageArray : public Image
{
public:
	ImageArray(const std::vector<std::string>& files, const ImageArrayProps& props = {});
	ImageArray(const ImageArrayProps& props);

	uint32_t getArrayLayers() const { return props.array_layers; }
	std::vector<std::string> getPaths() const { return paths; }

	static ImageData load(const std::vector<std::string>& files);

private:
	std::vector<std::string> paths;
};