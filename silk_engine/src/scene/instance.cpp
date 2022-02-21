#include "instance.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

void InstanceBatch::bind()
{
	instance->mesh->material->bind();
	descriptor_sets[0].bind();
	instance->mesh->vertex_array->bind();
	instance_buffer->bind(1);
}
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