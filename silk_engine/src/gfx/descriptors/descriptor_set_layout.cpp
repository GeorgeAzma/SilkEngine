#include "descriptor_set_layout.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

DescriptorSetLayout& DescriptorSetLayout::addBinding(uint32_t binding, vk::DescriptorType descriptor_type, vk::ShaderStageFlags shader_stages, size_t count)
{
	vk::DescriptorSetLayoutBinding layout_binding{};
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
	std::sort(bindings_vector.begin(), bindings_vector.end(), [](const vk::DescriptorSetLayoutBinding& l, const vk::DescriptorSetLayoutBinding& r)->bool { return l.binding < r.binding; });

	vk::DescriptorSetLayoutCreateInfo layout_info{};
	layout_info.bindingCount = bindings_vector.size();
	layout_info.pBindings = bindings_vector.data();
	Graphics::logical_device->createDescriptorSetLayout(layout_info);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	Graphics::logical_device->destroyDescriptorSetLayout(layout);
}
