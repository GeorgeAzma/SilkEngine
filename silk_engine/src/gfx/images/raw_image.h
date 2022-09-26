#pragma once

template<typename T = uint8_t>
class IRawImage
{
public:
	IRawImage() = default;
	void align4(); 
	void allocate() { pixels.resize(width * height * channels); }
	size_t size() const { return width * height * channels; }
	const T& operator()(uint32_t x, uint32_t y) const { return pixels[width * y + x]; }
	const T& operator()(uint32_t index) const { return pixels[index]; }
	const T& operator[](uint32_t index) const { return pixels[index]; }

public:
	uint32_t width = 0;
	uint32_t height = 0;
	uint8_t channels = 0;
	std::vector<T> pixels{};
};

template<typename T = uint8_t>
class RawImage : public IRawImage<T>
{
public:
	RawImage() = default;
};

template<>
class RawImage<uint8_t> : public IRawImage<uint8_t>
{
public:
	RawImage() = default;
	RawImage(std::string_view file);
	RawImage(const std::vector<std::string>& files); //Files must have same width/height/channels
	void load(std::string_view file);
	void load(const std::vector<std::string>& files); //Files must have same width/height/channels
	void save(std::string_view file); //Save as BMP
	void savePNG(std::string_view file);
};