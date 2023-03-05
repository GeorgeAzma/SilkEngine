#include "raw_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

template<typename T>
void IRawImage<T>::align4() //NOTE: This can be done in compute shader
{
	int old_channels = channels;
	channels = 4;
	std::vector<uint8_t> aligned_image(width * height * 4, 0);
	for (size_t i = 0; i < width * height; ++i)
	{
		memcpy(aligned_image.data() + i * 4, pixels.data() + i * old_channels, old_channels * sizeof(uint8_t));
		aligned_image[i * 4 + 3] = 255;
	}
	pixels = std::move(aligned_image);
}

RawImage<uint8_t>::RawImage(const path& file)
{
	load(file);
}

RawImage<uint8_t>::RawImage(std::span<const path> files)
{
	load(files);
}

void RawImage<uint8_t>::load(const path& file)
{
	path file_path = path("res/images") / file;

	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixel_data = stbi_load(file_path.string().c_str(), (int*)&width, (int*)&height, (int*)&channels, 0);
	SK_VERIFY(pixel_data, "Failed to load image: {}", file_path);

	allocate();
	memcpy(pixels.data(), pixel_data, pixels.size() * sizeof(uint8_t));

	stbi_image_free(pixel_data);

	SK_TRACE("Image Loaded: {}", file_path);
}

void RawImage<uint8_t>::load(std::span<const path> files)
{
	SK_VERIFY(files.size(), "You have to specify at least one filepath for loading images");

	if (files.size() == 1)
	{
		load(files[0]);
		return;
	}

	std::vector<path> paths(files.begin(), files.end());
	for (auto& file_path : paths)
		file_path = path("res/images") / file_path;

	RawImage<uint8_t> image_data{};
	image_data.load(paths[0]);
	if (image_data.channels == 3)
		image_data.align4();

	size_t size = image_data.size();
	width = image_data.width;
	height = image_data.height;
	channels = image_data.channels;
	pixels.resize(size * paths.size());
	memcpy(pixels.data(), image_data.pixels.data(), size * sizeof(uint8_t));

	for (size_t i = 1; i < paths.size(); ++i)
	{
		image_data.load(paths[i]);
		if (image_data.channels == 3)
			image_data.align4();

		SK_VERIFY(image_data.width == width
				  && image_data.height == height
				  && image_data.channels == channels,
				  "Error while loading images, couldn't load image at {0}. width, height and channel count should match in all the images", files[i]);

		memcpy(pixels.data() + i * size, image_data.pixels.data(), size * sizeof(uint8_t));
	}
}

void RawImage<uint8_t>::save(const path& file)
{
	stbi_write_bmp(file.string().c_str(), (int)width, (int)height, (int)channels, pixels.data());
	SK_TRACE("Image Saved: {}", file);
}

void RawImage<uint8_t>::savePNG(const path& file)
{
	stbi_write_png(file.string().c_str(), (int)width, (int)height, (int)channels, pixels.data(), 0);
	SK_TRACE("Image Saved: {}", file);
}