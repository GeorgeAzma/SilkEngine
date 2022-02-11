#pragma once

#include "image.h"

class Cubemap : public Image
{
public:
	Cubemap(const std::vector<std::string>& files, const CubemapProps& props = {});
	Cubemap(const CubemapProps& props);

	const std::vector<std::string>& getPaths() const { return paths; }

	static ImageData load(const std::vector<std::string>& files);

private:
	std::vector<std::string> paths;
};