#include "raw_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

RawImage<uint8_t>::RawImage(const fs::path& file, int align_channels)
{
	load(file, align_channels);
}

RawImage<uint8_t>::RawImage(std::span<const fs::path> files, int align_channels)
{
	load(files, align_channels);
}

void RawImage<uint8_t>::load(const fs::path& file, int align_channels)
{
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* pixel_data = stbi_load(file.string().c_str(), (int*)&width, (int*)&height, (int*)&channels, align_channels);
	SK_VERIFY(pixel_data, "Failed to load image: {}", file);

	allocate();
	memcpy(pixels.data(), pixel_data, pixels.size() * sizeof(uint8_t));

	stbi_image_free(pixel_data);

	SK_TRACE("Image Loaded: {}", file);
}

void RawImage<uint8_t>::load(std::span<const fs::path> files, int align_channels)
{
	SK_VERIFY(files.size(), "You have to specify at least one filepath for loading images");

	RawImage<uint8_t> image_data{};
	image_data.load(files[0], align_channels);

	size_t size = image_data.getSize();
	width = image_data.width;
	height = image_data.height;
	channels = image_data.channels;
	pixels.resize(size * files.size());
	memcpy(pixels.data(), image_data.pixels.data(), size * sizeof(uint8_t));

	for (size_t i = 1; i < files.size(); ++i)
	{
		image_data.load(files[i], align_channels);

		SK_VERIFY(image_data.width == width
				  && image_data.height == height
				  && image_data.channels == channels,
				  "Error while loading images, couldn't load image at {}. width, height and channel count should match in all the images", files[i]);

		memcpy(pixels.data() + i * size, image_data.pixels.data(), size * sizeof(uint8_t));
	}
}

void RawImage<uint8_t>::save(const fs::path& file)
{
	stbi_write_bmp(file.string().c_str(), (int)width, (int)height, (int)channels, pixels.data());
	SK_TRACE("Image Saved: {}", file);
}

void RawImage<uint8_t>::savePNG(const fs::path& file)
{
	stbi_write_png(file.string().c_str(), (int)width, (int)height, (int)channels, pixels.data(), 0);
	SK_TRACE("Image Saved: {}", file);
}