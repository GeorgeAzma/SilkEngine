#pragma once

#include "descriptor_set_layout.h"
#include "gfx/images/image.h"
#include "gfx/images/image_array.h"

class DescriptorSet : NonCopyable
{
public:
	DescriptorSet(const DescriptorSetLayout& layout);

	DescriptorSet& addBuffer(uint32_t binding, VkDescriptorBufferInfo buffer_info);
	DescriptorSet& addImage(uint32_t binding, const VkDescriptorImageInfo& descriptor_image_info);
	DescriptorSet& addImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& descriptor_image_info);
	void build();

	void bind();

	operator const DescriptorSetLayout& () const { return *layout; }

private:
	const DescriptorSetLayout* layout = nullptr;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	std::vector<VkWriteDescriptorSet> descriptor_writes;
};