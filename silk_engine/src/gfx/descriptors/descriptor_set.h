#pragma once

#include "descriptor_set_layout.h"
#include "gfx/images/image.h"
#include "gfx/images/image_array.h"

class DescriptorSet : NonCopyable
{
public:
	DescriptorSet& addBuffer(uint32_t binding, VkDescriptorBufferInfo buffer_info, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	DescriptorSet& addImage(uint32_t binding, const VkDescriptorImageInfo& descriptor_image_info, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	DescriptorSet& addImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& descriptor_image_info, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	void build();

	std::vector<VkWriteDescriptorSet>& getWrites() { return descriptor_writes; }
	void update();
	void update(const VkWriteDescriptorSet& descriptor_write);

	void bind(size_t first_set = 0);

	operator const VkDescriptorSet& () const { return descriptor_set; }
	const VkDescriptorSetLayout& getLayout() const { return *layout; }

private:
	shared <DescriptorSetLayout> layout;
	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	std::vector<VkWriteDescriptorSet> descriptor_writes;
	std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;
};