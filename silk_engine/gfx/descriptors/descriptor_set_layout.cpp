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
	size_t index = 0;
	bindings_vector.resize(bindings.size());
	for (const auto& binding : bindings) 
		bindings_vector[index++] = binding.second;
	
	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = bindings_vector.size();
	ci.pBindings = bindings_vector.data();
	layout = Graphics::logical_device->createDescriptorSetLayout(ci);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	Graphics::logical_device->destroyDescriptorSetLayout(layout);
}
