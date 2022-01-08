#include "descriptor_set.h"
#include "graphics.h"

DescriptorSet::DescriptorSet()
{
	VkDescriptorSetLayoutBinding layout_binding{};
	layout_binding.binding = 0;
	layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding.descriptorCount = 1;
	layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &layout_binding;

	Graphics::vulkanAssert(vkCreateDescriptorSetLayout(*Graphics::logical_device, &layout_info, nullptr, &layout));

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &layout;
}

DescriptorSet::~DescriptorSet()
{
	vkDestroyDescriptorSetLayout(*Graphics::logical_device, layout, nullptr);
}
