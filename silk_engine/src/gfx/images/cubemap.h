#pragma once

#include "image.h"

struct CubemapProps : public ImageProps
{
private:
	using ImageProps::depth;
	using ImageProps::layers;
};

class Cubemap : public Image
{
public:
	Cubemap(const std::vector<std::string>& files, const CubemapProps& props = {});
	Cubemap(const CubemapProps& props);
};