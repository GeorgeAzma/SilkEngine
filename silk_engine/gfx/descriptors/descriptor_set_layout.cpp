#include "descriptor_set_layout.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

DescriptorSetLayout::~DescriptorSetLayout()
{
	RenderContext::getLogicalDevice().destroyDescriptorSetLayout(layout);
}

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
	layout = RenderContext::getLogicalDevice().createDescriptorSetLayout(ci);
}

shared<DescriptorSetLayout> DescriptorSetLayout::get(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	auto layout = descriptor_set_layouts.find(bindings);
	if (layout != descriptor_set_layouts.end())
		return layout->second;

	return add(bindings);
}

shared<DescriptorSetLayout> DescriptorSetLayout::add(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	shared<DescriptorSetLayout> new_layout = makeShared<DescriptorSetLayout>();
	for (size_t i = 0; i < bindings.size(); ++i)
		new_layout->addBinding(bindings[i].binding, bindings[i].descriptorType, bindings[i].stageFlags, bindings[i].descriptorCount);
	new_layout->build();

	descriptor_set_layouts[bindings] = new_layout;

	return new_layout;
}