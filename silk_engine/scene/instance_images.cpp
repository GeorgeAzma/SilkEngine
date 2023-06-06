#include "instance_images.h"
#include "silk_engine/gfx/images/image.h"
#include "silk_engine/gfx/debug/debug_renderer.h"

InstanceImages::InstanceImages(uint32_t max_images)
	: max_images(max_images ? max_images : DebugRenderer::MAX_IMAGE_SLOTS) {}

uint32_t InstanceImages::add(const std::vector<shared<Image>>& new_images)
{
	if (new_images.size() == 1)
		return add(new_images[0]);

	enum Action : uint8_t { Add, Replace, Use };

	std::array<Action, 64> actions;

	size_t j = 0;
up:
	for (size_t i = 0; i < new_images.size(); ++i)
	{
		if (i + j >= images.size())
			actions[i] = Add;
		else if (images[i + j].get() == new_images[i].get())
			actions[i] = Use;
		else if (image_owners[i + j] == 0)
			actions[i] = Replace;
		else
		{
			if (++j > (max_images - new_images.size()))
				return UINT32_MAX;
			goto up;
		}
	}

	for (size_t i = 0; i < new_images.size(); ++i)
	{
		switch (actions[i])
		{
		case Add:
			images.emplace_back(new_images[i]);
			image_owners.emplace_back(1);
			break;
		case Use:
			++image_owners[i + j];
			break;
		case Replace:
			images[i + j] = new_images[i];
			++image_owners[i + j];
			break;
		}
	}

	return j;
}

uint32_t InstanceImages::add(const shared<Image>& new_image)
{
	for (size_t i = 0; i < images.size(); ++i)
	{
		if (images[i] == new_image)
		{
			++image_owners[i];
			return i;
		}
	}
	for (size_t i = 0; i < images.size(); ++i)
	{
		if (!image_owners[i])
		{
			images[i] = new_image;
			++image_owners[i];
			return i;
		}
	}
	if (images.size() < max_images)
	{
		images.emplace_back(new_image);
		image_owners.emplace_back(1);
		return images.size() - 1;
	}
	return std::numeric_limits<uint32_t>::max();
}

void InstanceImages::remove(size_t index, size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		SK_VERIFY(image_owners[index + i] > 0, "Couldn't remove image from instance images");
		--image_owners[index + i];
	}
}

const std::vector<shared<Image>>& InstanceImages::getImages() const
{
	return images;
}

std::vector<VkDescriptorImageInfo> InstanceImages::getDescriptorImageInfos() const
{
	std::vector<VkDescriptorImageInfo> image_descriptor_infos(max_images);
	for (size_t i = 0; i < images.size(); ++i)
		image_descriptor_infos[i] = images[i]->getDescriptorInfo();
	for (size_t i = images.size(); i < image_descriptor_infos.size(); ++i)
		image_descriptor_infos[i] = DebugRenderer::getWhiteImage()->getDescriptorInfo();
	return image_descriptor_infos;
}
