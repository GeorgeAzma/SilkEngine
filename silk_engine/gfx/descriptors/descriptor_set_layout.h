#pragma once

class DescriptorSetLayout : NonCopyable
{
	friend class DescriptorSet;
	friend class Resources;

public:
	~DescriptorSetLayout();

	DescriptorSetLayout& addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, size_t count = 1);
	void build();

	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return bindings_vector; }
	operator const VkDescriptorSetLayout& () const { return layout; }

private:
	VkDescriptorSetLayout layout = nullptr;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorSetLayoutBinding> bindings_vector;
};