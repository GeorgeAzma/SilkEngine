#pragma once

class DescriptorSetLayout
{
public:
	~DescriptorSetLayout();

	DescriptorSetLayout& addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlagBits shader_stages, size_t count = 1);
	void build();

	operator const VkDescriptorSetLayout& () { return layout; }

private:
	VkDescriptorSetLayout layout{};
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
};