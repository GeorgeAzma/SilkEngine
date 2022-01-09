#pragma once

#include "descriptor_set_layout.h"

class DescriptorSet
{
public:
	DescriptorSet(const DescriptorSetLayout& layout, size_t count = 1);

	DescriptorSet& addBuffer(uint32_t binding, VkDescriptorBufferInfo buffer_info);
	void build();

	void bind(size_t index = 0);

private:
	const DescriptorSetLayout* layout = nullptr;
	std::vector<VkDescriptorSet> descriptor_sets;
	std::vector<VkWriteDescriptorSet> descriptor_writes;
};