#include "instance.h"
#include "gfx/graphics.h"
#include "gfx/window/swap_chain.h"
#include "gfx/devices/logical_device.h"

bool InstanceData::operator==(const InstanceData& other) const
{
	return transform == other.transform
		&& image_index == other.image_index
		&& color == color;
}

bool RenderedInstance::operator==(const RenderedInstance& other) const
{
	return (*mesh == *other.mesh);
}

InstanceBatch::~InstanceBatch()
{
	Graphics::logical_device->waitIdle();
}

void InstanceBatch::bind()
{
	instance->material->pipeline->bind();
	for (auto& descriptor_set : descriptor_sets)
		descriptor_set.second.bind(descriptor_set.first);
	instance->mesh->vertex_array->bind();
	instance_buffer->bind(1);
}

uint32_t InstanceBatch::addImages(const std::vector<shared<Image2D>>& new_images)
{
	SK_ASSERT(new_images.size(), "You should specify more than 0 images to add");
	bool found = false;
	for (int64_t i = 0; i < ((int64_t)images.size() - ((int64_t)new_images.size() - 1)); ++i)
	{
		found = true;
		for (size_t j = 0; j < new_images.size(); ++j)
		{
			if (images[i + j].get() != new_images[j].get())
			{
				if (!image_owners[i + j])
				{
					images_need_update = true;
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

	if (images.size() + new_images.size() > Graphics::MAX_IMAGE_SLOTS)
		return UINT32_MAX;

	size_t image_index = images.size();
	images.insert(images.end(), new_images.begin(), new_images.end());
	images_need_update = true;
	
	for (size_t j = 0; j < new_images.size(); ++j)
		++image_owners[image_index + j];

	return image_index;
}

void InstanceBatch::removeImages(size_t index, size_t count)
{
	for (size_t i = 0; i < count; ++i)
		--image_owners[index + i];
}

bool InstanceBatch::operator==(const RenderedInstance& instance) const
{
	return *this->instance == instance;
}
