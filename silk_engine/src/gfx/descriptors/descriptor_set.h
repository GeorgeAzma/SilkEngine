#pragma once

#include "descriptor_set_layout.h"
#include "gfx/images/image.h"
#include "gfx/images/image_array.h"

class DescriptorSet : NonMovable
{
public:
	DescriptorSet() = default;
	DescriptorSet(const DescriptorSet & other);
	DescriptorSet& operator=(const DescriptorSet & other);

	DescriptorSet& add(uint32_t binding, uint32_t count, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	void build();

	void update() const;

	void bind(size_t first_set = 0) const;

	bool wasUpdated() const { return updated; }
	const VkDescriptorSetLayout& getLayout() const { return *layout; }
	std::vector<VkWriteDescriptorSet>& getWrites() { return write_descriptor_sets; }
	operator const VkDescriptorSet& () const { return descriptor_set; }

	void setImageInfo(size_t write_index, const std::vector<VkDescriptorImageInfo>& image_info);
	void setBufferInfo(size_t write_index, const std::vector<VkDescriptorBufferInfo>& buffer_info);

private:
	VkDescriptorSet descriptor_set;
	shared<DescriptorSetLayout> layout;
	std::vector<VkWriteDescriptorSet> write_descriptor_sets; 
	std::vector<std::vector<VkDescriptorImageInfo>> image_infos;
	std::vector<std::vector<VkDescriptorBufferInfo>> buffer_infos;
	std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;
	mutable bool updated = false;
};