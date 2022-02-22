#include "instance.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"

void InstanceBatch::bind()
{
	instance->mesh->material->pipeline->bind();
	for (auto& descriptor_set : descriptor_sets[Graphics::swap_chain->getImageIndex()])
		descriptor_set.second.bind(descriptor_set.first);
	instance->mesh->vertex_array->bind();
	instance_buffer->bind(1);
}
uint32_t InstanceBatch::addImages(const std::vector<shared<Image>>& new_images)
{
	SK_ASSERT(new_images.size(), "You should specify more than 0 images to add");
	bool found = false;
	for (size_t i = 0; i < (images.size() - (new_images.size() - 1)); ++i)
	{
		found = true;
		for (size_t j = 0; j < new_images.size(); ++j)
		{
			if (!image_owners[i + j])
				images[i + j] = new_images[j];
			if (images[i + j].get() != new_images[j].get())
				found = false;
		}
		if (found)
		{
			for (size_t j = 0; j < new_images.size(); ++j)
				++image_owners[i + j];
			return i;
		}
	}

	if (images.size() + new_images.size() > Graphics::MAX_IMAGE_SLOTS)
		return UINT32_MAX;

	size_t image_index = images.size();
	images.insert(images.end(), new_images.begin(), new_images.end());
	images_need_update = true;
	
	for (size_t j = 0; j < new_images.size(); ++j)
		++image_owners[image_index + j];

	return image_index;
}