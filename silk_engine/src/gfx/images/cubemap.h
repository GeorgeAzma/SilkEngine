#pragma once

#include "image.h"

class Cubemap : public Image
{
public:
	Cubemap(const std::vector<std::string>& files, const CubemapProps& props = {});
	Cubemap(const CubemapProps& props);
};