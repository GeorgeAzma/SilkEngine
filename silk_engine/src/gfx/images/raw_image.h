#pragma once

struct RawImage
{
public:
	RawImage() = default;
	RawImage(std::string_view file);
	void load(std::string_view file);
	void load(const std::vector<std::string>& files); //Loads files with same width/height/channels and places their data in one big array
	void save(std::string_view file); //Save as BMP
	void savePNG(std::string_view file);
	void align4(); 
	void allocate() { pixels.resize(width * height * channels); }
	size_t size() const { return width * height * channels * sizeof(uint8_t); }

public:
	uint32_t width = 0;
	uint32_t height = 0;
	uint8_t channels = 0;
	std::vector<uint8_t> pixels{};
};