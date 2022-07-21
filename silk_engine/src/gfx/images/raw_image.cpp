#include "raw_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

RawImage::RawImage(std::string_view file)
{
	load(file);
}

void RawImage::load(std::string_view file)
{
	std::string path = std::string("data/images/") + file.data();

	stbi_uc* pixel_data = stbi_load(path.c_str(), (int*)&width, (int*)&height, (int*)&channels, 0);
	SK_ASSERT(pixel_data, "Failed to load image: {0}", path);

	pixels.resize(size());
	std::memcpy(pixels.data(), pixel_data, pixels.size() * sizeof(uint8_t));

	stbi_image_free(pixel_data);

	SK_TRACE("Image Loaded: {0}", path);
}

void RawImage::load(const std::vector<std::string>& files)
{
	SK_ASSERT(files.size(), "You have to specify at least one filepath for loading images");

	if (files.size() == 1)
	{
		load(files[0]);
		return;
	}

	std::vector<std::string> paths = files;
	for (auto& path : paths)
		path = "data/images/" + path;

	RawImage image_data{};
	image_data.load(paths[0]);
	if (image_data.channels == 3)
		image_data.align4();

	size_t size = image_data.size();
	width = image_data.width;
	height = image_data.height;
	channels = image_data.channels;
	pixels.resize(size * paths.size());
	std::memcpy(pixels.data(), image_data.pixels.data(), size);

	for (size_t i = 1; i < paths.size(); ++i)
	{
		image_data.load(paths[i]);
		if (image_data.channels == 3)
			image_data.align4();

		SK_ASSERT(image_data.width == width
				  && image_data.height == height
				  && image_data.channels == channels,
				  "Error while loading images, couldn't load image at {0}. width, height and channel count should match in all the images", files[i]);

		std::memcpy(pixels.data() + i * size / sizeof(uint8_t), image_data.pixels.data(), size);
	}
}

void RawImage::save(std::string_view file)
{
	stbi_write_bmp(file.data(), (int)width, (int)height, (int)channels, pixels.data());
	SK_TRACE("Image Saved: {0}", file.data());
}

void RawImage::savePNG(std::string_view file)
{
	stbi_write_png(file.data(), (int)width, (int)height, (int)channels, pixels.data(), 0);
	SK_TRACE("Image Saved: {0}", file.data());
}

void RawImage::align4() //NOTE: This can be done in compute shader
{
	int old_channels = channels;
	channels = 4;
	std::vector<uint8_t> aligned_image(width * height * 4, 0);
	for (size_t i = 0; i < width * height; ++i)
	{
		std::memcpy(aligned_image.data() + i * 4, pixels.data() + i * old_channels, old_channels * sizeof(uint8_t));
		aligned_image[i * 4 + 3] = 255;
	}
	pixels = std::move(aligned_image);
}