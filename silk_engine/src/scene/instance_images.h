#pragma once

#include "gfx/images/image2D.h"
#include "gfx/descriptors/descriptor_set.h"

class InstanceImages
{
public:
	InstanceImages(uint32_t max_images = 0);
	
	///@return texture index, UINT32_MAX if out of space
	uint32_t add(const std::vector<shared<Image2D>>& new_images);
	void remove(size_t index, size_t count = 1);
	uint32_t available() const { return max_images - images.size(); }
	const std::vector<shared<Image2D>>& getImages() const { return images; }
	std::vector<VkDescriptorImageInfo> getDescriptorImageInfos() const;

private:
	uint32_t max_images = 0;
	std::vector<shared<Image2D>> images;
	std::vector<uint32_t> image_owners;
	bool need_update = true;
};