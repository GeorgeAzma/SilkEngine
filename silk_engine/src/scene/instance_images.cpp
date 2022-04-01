#include "instance_images.h"
#include "resources.h"
#include "gfx/graphics.h"

InstanceImages::InstanceImages(uint32_t max_images)
{
	this->max_images = max_images ? max_images : Graphics::MAX_IMAGE_SLOTS;
	image_owners.resize(this->max_images);
	images.reserve(this->max_images);
}

uint32_t InstanceImages::add(const std::vector<shared<Image2D>>& new_images)
{
	SK_ASSERT(new_images.size(), "You should specify more than 0 images to add");

	if (images.size() + new_images.size() > max_images)
		return UINT32_MAX;

	bool found = false;
	size_t s = images.size() - (new_images.size() - 1);
	for (size_t i = 0; i < s; ++i)
	{
		found = true;
		for (size_t j = 0; j < new_images.size(); ++j)
		{
			if (images[i + j].get() != new_images[j].get())
			{
				if (!image_owners[i + j])
				{
					need_update = true;
					images[i + j] = new_images[j];
				}
				else found = false;
			}
		}
		if (found)
		{
			for (size_t j = 0; j < new_images.size(); ++j)
				++image_owners[i + j];
			return i;
		}
	}

	size_t image_index = images.size();
	images.insert(images.end(), new_images.begin(), new_images.end());
	need_update = true;

	for (size_t j = 0; j < new_images.size(); ++j)
		++image_owners[image_index + j];

	return image_index;
}

void InstanceImages::remove(size_t index, size_t count)
{
	for (size_t i = 0; i < count; ++i)
		--image_owners[index + i];
}

void InstanceImages::updateDescriptorSet(DescriptorSet& descriptor_set, uint32_t write_index)
{
	if (need_update)
	{
		std::vector<VkDescriptorImageInfo> descriptor_images(max_images, Resources::white_image->getDescriptorInfo());
		for (size_t i = 0; i < images.size(); ++i)
			descriptor_images[i] = *images[i];

		descriptor_set.setImageInfo(write_index, descriptor_images);
		need_update = false;
	}
}
