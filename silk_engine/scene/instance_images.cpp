#include "instance_images.h"
#include "resources.h"
#include "gfx/renderer.h"

InstanceImages::InstanceImages(uint32_t max_images)
{
	this->max_images = max_images ? max_images : Renderer::MAX_IMAGE_SLOTS;
}

uint32_t InstanceImages::add(const std::vector<shared<Image>>& new_images)
{
	SK_ASSERT(new_images.size(), "You should specify more than 0 images to add");

	enum Action : uint8_t { Add, Replace, Use };

	Action* actions = new Action[new_images.size()]{};

	size_t j = 0;
up:
	for (size_t i = 0; i < new_images.size(); ++i)
	{
		if (i + j >= images.size())
			actions[i] = Add;
		else if (images[i + j] == new_images[i])
			actions[i] = Use;
		else if (image_owners[i + j] == 0)
			actions[i] = Replace;
		else
		{
			++j;
			if (j > (max_images - new_images.size()))
				return UINT32_MAX;
			goto up;
		}
	}

	bool need_update = false;
	for (size_t i = 0; i < new_images.size(); ++i)
	{
		switch (actions[i])
		{
		case Add:
			images.emplace_back(new_images[i]);
			image_owners.emplace_back(1);
			need_update = true;
			break;
		case Use:
			++image_owners[i + j];
			break;
		case Replace:
			images[i + j] = new_images[i];
			++image_owners[i + j];
			need_update = true;
			break;
		}
	}

	delete[] actions;
	return j;
}

void InstanceImages::remove(size_t index, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		SK_ASSERT(image_owners[index + i] > 0, "Couldn't remove image from instance images");
		--image_owners[index + i];
	}
}

std::vector<VkDescriptorImageInfo> InstanceImages::getDescriptorImageInfos() const
{
	std::vector<VkDescriptorImageInfo> image_descriptor_infos(max_images);
	for (size_t i = 0; i < images.size(); ++i)
		image_descriptor_infos[i] = images[i]->getDescriptorInfo();
	for (size_t i = images.size(); i < image_descriptor_infos.size(); ++i)
		image_descriptor_infos[i] = Resources::white_image->getDescriptorInfo();
	return image_descriptor_infos;
}
