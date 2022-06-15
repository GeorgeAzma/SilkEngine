#include "dynamic_image.h"

template<typename T>
void DynamicImage<T>::create(const ImageProps& props)
{
	image_data = props.width * props.height * props.depth * props.layers;
	Image::create(props);
}