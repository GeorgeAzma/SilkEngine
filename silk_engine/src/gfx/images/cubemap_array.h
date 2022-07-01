#pragma once

#include "image.h"

struct CubemapArrayProps : public ImageProps
{
private:
	using ImageProps::depth;
};

class CubemapArray : public Image
{
public:
	CubemapArray(const std::vector<std::string>& files, const CubemapArrayProps& props = {});
	CubemapArray(const CubemapArrayProps& props);
};