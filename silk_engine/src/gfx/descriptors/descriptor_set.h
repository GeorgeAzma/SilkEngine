#pragma once

#include "descriptor_set_layout.h"
#include "gfx/images/image.h"
#include "gfx/images/image_array.h"
#include "gfx/descriptors/write_descriptor_set.h"

class DescriptorSet : NonCopyable
{
public:
	DescriptorSet& addBuffers(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& descriptor_buffer_infos, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	DescriptorSet& addImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& descriptor_image_infos, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	void build();

	void update();
	void update(const std::vector<shared<WriteDescriptorSet>>& write_descriptor_sets);

	void bind(size_t first_set = 0);

	operator const VkDescriptorSet& () const { return descriptor_set; }
	const VkDescriptorSetLayout& getLayout() const { return *layout; }
	std::vector<shared<WriteDescriptorSet>>& getWrites() { return write_descriptor_sets; }

private:
	VkDescriptorSet descriptor_set;
	shared<DescriptorSetLayout> layout;
	std::vector<shared<WriteDescriptorSet>> write_descriptor_sets;
	std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;
};