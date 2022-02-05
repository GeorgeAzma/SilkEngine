#include "descriptor_set.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/descriptors/descriptor_pool.h"
#include "scene/resources.h"

DescriptorSet& DescriptorSet::addBuffers(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& descriptor_buffer_infos, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	WriteDescriptorSetProps write_descriptor_set_props{};
	write_descriptor_set_props.descriptor_type = descriptor_type;
	write_descriptor_set_props.dst_binding = binding;
	write_descriptor_set_props.dst_array_element = 0;
	write_descriptor_sets.emplace_back(makeShared<WriteDescriptorSet>(write_descriptor_set_props, descriptor_buffer_infos));

	VkDescriptorSetLayoutBinding descriptor_layout_binding{};
	descriptor_layout_binding.binding = binding;
	descriptor_layout_binding.descriptorCount = descriptor_buffer_infos.size();
	descriptor_layout_binding.descriptorType = descriptor_type;
	descriptor_layout_binding.stageFlags = stage_flags;
	descriptor_layout_bindings.emplace_back(std::move(descriptor_layout_binding));

	return *this;
}

DescriptorSet& DescriptorSet::addImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& descriptor_image_infos, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	WriteDescriptorSetProps write_descriptor_set_props{};
	write_descriptor_set_props.descriptor_type = descriptor_type;
	write_descriptor_set_props.dst_binding = binding;
	write_descriptor_set_props.dst_array_element = 0;
	write_descriptor_sets.emplace_back(makeShared<WriteDescriptorSet>(write_descriptor_set_props, descriptor_image_infos));

	VkDescriptorSetLayoutBinding descriptor_layout_binding{};
	descriptor_layout_binding.binding = binding;
	descriptor_layout_binding.descriptorCount = descriptor_image_infos.size();
	descriptor_layout_binding.descriptorType = descriptor_type;
	descriptor_layout_binding.stageFlags = stage_flags;
	descriptor_layout_bindings.emplace_back(std::move(descriptor_layout_binding));

	return *this;
}

void DescriptorSet::build()
{
	layout = Resources::getDescriptorSetLayout(descriptor_layout_bindings);

	VkDescriptorSetAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorPool = *Graphics::descriptor_pool;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &layout->layout;
	
	Graphics::vulkanAssert(vkAllocateDescriptorSets(*Graphics::logical_device, &allocation_info, &descriptor_set));

	for (auto& write : write_descriptor_sets)
		write->setDstDescriptorSet(descriptor_set);

	update();
}

void DescriptorSet::update()
{
	update(write_descriptor_sets);
}

void DescriptorSet::update(const std::vector<shared<WriteDescriptorSet>>& write_descriptor_sets)
{
	std::vector<VkWriteDescriptorSet> writes(write_descriptor_sets.size());
	for (size_t i = 0; i < write_descriptor_sets.size(); ++i)
		writes[i] = *write_descriptor_sets[i];
	vkUpdateDescriptorSets(*Graphics::logical_device, writes.size(), writes.data(), 0, nullptr);
}

void DescriptorSet::bind(size_t first_set)
{
	vkCmdBindDescriptorSets(Graphics::active.command_buffer, Graphics::active.bind_point, Graphics::active.pipeline_layout, first_set, 1, &descriptor_set, 0, nullptr);
}