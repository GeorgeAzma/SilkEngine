#include "descriptor_set_layout.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

DescriptorSetLayout& DescriptorSetLayout::addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags shader_stages, size_t count)
{
	VkDescriptorSetLayoutBinding layout_binding{};
	layout_binding.binding = binding;
	layout_binding.descriptorType = descriptor_type;
	layout_binding.descriptorCount = count;
	layout_binding.stageFlags = shader_stages;
	layout_binding.pImmutableSamplers = nullptr;

	bindings[binding] = std::move(layout_binding);

	return *this;
}

void DescriptorSetLayout::build()
{
	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

	size_t index = 0;
	bindings_vector.resize(bindings.size());
	for (const auto& binding : bindings) 
		bindings_vector[index++] = binding.second;

	layout_info.bindingCount = bindings_vector.size();
	layout_info.pBindings = bindings_vector.data();

	Graphics::vulkanAssert(vkCreateDescriptorSetLayout(*Graphics::logical_device, &layout_info, nullptr, &layout));
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(*Graphics::logical_device, layout, nullptr);
}
