#include "bitmap.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void Bitmap::load(std::string_view file)
{
	std::string path = std::string("data/images/") + file.data();

	stbi_uc* pixels = stbi_load(path.c_str(), (int*)&width, (int*)&height, (int*)&channels, 0);
	SK_ASSERT(pixels, "Failed to load image: {0}", path);

	data.resize(size());
	std::memcpy(data.data(), pixels, data.size() * sizeof(uint8_t));

	stbi_image_free(pixels);

	SK_TRACE("Image Loaded: {0}", path);
}

void Bitmap::load(const std::vector<std::string>& files)
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

	Bitmap image_data{};
	image_data.load(paths[0]);
	if (image_data.channels == 3)
		image_data.align4();

	size_t size = image_data.size();
	width = image_data.width;
	height = image_data.height;
	channels = image_data.channels;
	data.resize(size * paths.size());
	std::memcpy(data.data(), image_data.data.data(), size);

	for (size_t i = 1; i < paths.size(); ++i)
	{
		image_data.load(paths[i]);
		if (image_data.channels == 3)
			image_data.align4();

		SK_ASSERT(image_data.width == width
				  && image_data.height == height
				  && image_data.channels == channels,
				  "Error while loading images, couldn't load image at {0}. width, height and channel count should match in all the images", files[i]);

		std::memcpy(data.data() + i * size / sizeof(uint8_t), image_data.data.data(), size);
	}
}

void Bitmap::save(std::string_view file)
{
	stbi_write_bmp(file.data(), (int)width, (int)height, (int)channels, data.data());
	SK_TRACE("Image Saved: {0}", file.data());
}

void Bitmap::savePNG(std::string_view file)
{
	stbi_write_png(file.data(), (int)width, (int)height, (int)channels, data.data(), 0);
	SK_TRACE("Image Saved: {0}", file.data());
}

void Bitmap::align4() //NOTE: This can be done in compute shader
{
	int old_channels = channels;
	channels = 4;
	std::vector<uint8_t> aligned_image(width * height * 4, 0);
	for (size_t i = 0; i < width * height; ++i)
	{
		std::memcpy(aligned_image.data() + i * 4, data.data() + i * old_channels, old_channels * sizeof(uint8_t));
		aligned_image[i * 4 + 3] = 255;
	}
	data = std::move(aligned_image);
}