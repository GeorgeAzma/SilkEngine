#pragma once

class Image;

class InstanceImages
{
public:
	InstanceImages(uint32_t max_images = 0);
	
	///@return texture index, UINT32_MAX if out of space
	uint32_t add(const std::vector<shared<Image>>& new_images);
	uint32_t add(const shared<Image>& new_image);
	void remove(size_t index, size_t count = 1);
	uint32_t available() const { return max_images - images.size(); }
	const std::vector<shared<Image>>& getImages() const;
	std::vector<VkDescriptorImageInfo> getDescriptorImageInfos() const;
	void clear() { images.clear(); image_owners.clear(); }

private:
	uint32_t max_images = 0;
	std::vector<shared<Image>> images;
	std::vector<uint32_t> image_owners;
};