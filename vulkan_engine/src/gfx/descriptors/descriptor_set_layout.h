#pragma once

class DescriptorSetLayout : NonCopyable
{
	friend class DescriptorSet;
public:
	~DescriptorSetLayout();

	DescriptorSetLayout& addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, size_t count = 1);
	void build();

	operator const VkDescriptorSetLayout& () { return layout; }

private:
	VkDescriptorSetLayout layout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
};