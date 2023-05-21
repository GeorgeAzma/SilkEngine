#pragma once

template <typename T>
class IRawImage
{
public:
	IRawImage() = default;
	void allocate() { pixels.resize(getPixelCount()); }
	size_t getPixelCount() const { return width * height * channels; }
	size_t getSize() const { return getPixelCount() * sizeof(T); }
	T& operator[](size_t index) { return pixels[index]; }
	//T& operator[](size_t x, size_t y) { return pixels[width * y + x]; }
	const T& operator[](size_t index) const { return pixels[index]; }
	//const T& operator[](size_t x, size_t y) const { return pixels[width * y + x]; }

public:
	uint32_t width = 0;
	uint32_t height = 0;
	uint8_t channels = 0;
	std::vector<T> pixels{};
};

template <typename T = uint8_t>
class RawImage : public IRawImage<T>
{
public:
	RawImage() = default;
	RawImage(const fs::path& file, int align_channels = 0) {}
	RawImage(std::span<const fs::path> files, int align_channels = 0) {} //Files must have same width/height/channels
	
	void load(const fs::path& file, int align_channels = 0) {}
	void load(std::span<const fs::path> files, int align_channels = 0) {} //Files must have same width/height/channels
	void save(const fs::path& file) {} //Save as BMP
	void savePNG(const fs::path& file) {}
};

template<>
class RawImage<uint8_t> : public IRawImage<uint8_t>
{
public:
	RawImage() = default;
	RawImage(const fs::path& file, int align_channels = 0);
	RawImage(std::span<const fs::path> files, int align_channels = 0); //Files must have same width/height/channels

	void load(const fs::path& file, int align_channels = 0);
	void load(std::span<const fs::path> files, int align_channels = 0); //Files must have same width/height/channels
	void save(const fs::path& file); //Save as BMP
	void savePNG(const fs::path& file);
};