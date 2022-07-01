#pragma once

struct Bitmap
{
	uint32_t width;
	uint32_t height;
	uint8_t channels;
	std::vector<uint8_t> pixels;

public:
	void load(std::string_view file);
	void load(const std::vector<std::string>& files); //Loads files with same width/height/channels and places their data in one big array
	void save(std::string_view file);
	void savePNG(std::string_view file);
	void align4();
	void allocate() { pixels.resize(width * height * channels); }
	size_t size() const { return width * height * channels * sizeof(uint8_t); }
	const uint8_t* data() const { return pixels.data(); }
	uint8_t* data() { return pixels.data(); }
};