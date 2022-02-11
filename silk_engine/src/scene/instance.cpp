#include "instance.h"
#include "gfx/graphics.h"

uint32_t InstanceBatch::addImages(const std::vector<shared<Image>>& new_images)
{
	const auto it = std::search(images.begin(), images.end(), new_images.begin(), new_images.end());
	if (it != images.cend())
		return size_t(it - images.begin());

	if (images.size() + new_images.size() > Graphics::MAX_IMAGE_SLOTS)
		return UINT32_MAX;

	size_t image_index = images.size();
	images.insert(images.end(), new_images.begin(), new_images.end());

	return image_index;
}