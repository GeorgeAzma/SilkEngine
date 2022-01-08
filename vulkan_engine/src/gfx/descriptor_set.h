#pragma once

class DescriptorSet
{
public:
	DescriptorSet();
	~DescriptorSet();

private:
	VkDescriptorSetLayout layout{};
	VkPipelineLayout pipeline_layout;
};