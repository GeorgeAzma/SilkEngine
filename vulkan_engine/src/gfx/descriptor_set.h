#pragma once

class DescriptorSet
{
public:
	DescriptorSet(VkDescriptorSetLayout layout, size_t count = 1);

	void bind(size_t index = 0);

private:
	std::vector<VkDescriptorSet> descriptor_sets;
};