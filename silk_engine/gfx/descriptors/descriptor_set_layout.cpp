#include "descriptor_set_layout.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

DescriptorSetLayout::DescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	for (const auto& binding : bindings)
	{
		if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
			dynamic_descriptor_count += binding.descriptorCount;
		this->bindings.emplace(binding.binding, binding);
	}
	
	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = bindings.size();
	ci.pBindings = bindings.data();
	layout = RenderContext::getLogicalDevice().createDescriptorSetLayout(ci);
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	RenderContext::getLogicalDevice().destroyDescriptorSetLayout(layout);
}