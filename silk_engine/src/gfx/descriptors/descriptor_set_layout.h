#pragma once

#include <vulkan/vulkan.hpp>

class DescriptorSetLayout : NonCopyable
{
	friend class DescriptorSet;
	friend class Resources;
public:
	~DescriptorSetLayout();

	DescriptorSetLayout& addBinding(uint32_t binding, vk::DescriptorType descriptor_type, vk::ShaderStageFlags shader_stages = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, size_t count = 1);
	void build();

	const std::vector<vk::DescriptorSetLayoutBinding>& getBindings() const { return bindings_vector; }
	operator const vk::DescriptorSetLayout& () const { return layout; }

private:
	vk::DescriptorSetLayout layout;
	std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorSetLayoutBinding> bindings_vector;
};