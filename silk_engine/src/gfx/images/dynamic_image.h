#pragma once

#include "image.h"

template<typename T>
class DynamicImage : public Image
{
public:
	void update(uint32_t base_layer = 0, uint32_t layers = 1);
	T& operator[](size_t index) { return image_data[index]; }
	T* data() { return image_data.data(); }
	T& at(size_t x, size_t y) { return image_data[y * getWidth() + x]; }
	size_t getSize() const { return image_data.size(); }
	std::vector<T>::iterator begin() { return image_data.begin(); }
	std::vector<T>::iterator end() { return image_data.end(); }
	std::vector<T>::const_iterator cbegin() const { return image_data.cbegin(); }
	std::vector<T>::const_iterator cend() const { return image_data.cend(); }

private:
	void create(const ImageProps& props);

private:
	std::vector<T> image_data;

private:
	using Image::isMapped;
	using Image::setData;
	using Image::getData;
	using Image::copyFromBuffer;
	using Image::reallocate;
	using Image::map;
	using Image::unmap;
};

template<typename T>
inline void DynamicImage<T>::update(uint32_t base_layer, uint32_t layers)
{
	setData(image_data.data(), base_layer, layers);
}
