#include "descriptor_set.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/descriptors/descriptor_pool.h"
#include "scene/resources.h"

DescriptorSet& DescriptorSet::add(uint32_t binding, uint32_t count, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	buffer_infos.push_back({});
	image_infos.push_back({});

	VkWriteDescriptorSet write_descriptor{};
	write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor.descriptorCount = count;
	write_descriptor.descriptorType = descriptor_type;
	write_descriptor.dstArrayElement = 0;
	write_descriptor.dstBinding = binding;
	write_descriptor_sets.emplace_back(std::move(write_descriptor));

	VkDescriptorSetLayoutBinding descriptor_layout_binding{};
	descriptor_layout_binding.binding = binding;
	descriptor_layout_binding.descriptorCount = count;
	descriptor_layout_binding.descriptorType = descriptor_type;
	descriptor_layout_binding.stageFlags = stage_flags;
	descriptor_layout_bindings.emplace_back(std::move(descriptor_layout_binding));

	return *this;
}

void DescriptorSet::build()
{
	if (descriptor_layout_bindings.empty())
		return;

	layout = Resources::getDescriptorSetLayout(descriptor_layout_bindings);

	VkDescriptorSetAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorPool = *Graphics::descriptor_pool;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &layout->layout;
	
	Graphics::vulkanAssert(vkAllocateDescriptorSets(*Graphics::logical_device, &allocation_info, &descriptor_set));

	for (auto& write : write_descriptor_sets)
		write.dstSet = descriptor_set;
}

void DescriptorSet::update() const
{
	vkUpdateDescriptorSets(*Graphics::logical_device, write_descriptor_sets.size(), write_descriptor_sets.data(), 0, nullptr);
}

void DescriptorSet::bind(size_t first_set) const
{
	vkCmdBindDescriptorSets(Graphics::active.command_buffer, Graphics::active.bind_point, Graphics::active.pipeline_layout, first_set, 1, &descriptor_set, 0, nullptr);
}

void DescriptorSet::setImageInfo(size_t write_index, const std::vector<VkDescriptorImageInfo>& image_info)
{
	SK_ASSERT(image_info.size() == write_descriptor_sets[write_index].descriptorCount, "Invalid image_info size: {0}, should be {1}", image_info.size(), write_descriptor_sets[write_index].descriptorCount);
	this->image_infos[write_index] = image_info;
	write_descriptor_sets[write_index].pImageInfo = this->image_infos[write_index].data();
}

void DescriptorSet::setBufferInfo(size_t write_index, const std::vector<VkDescriptorBufferInfo>& buffer_info)
{
	SK_ASSERT(buffer_info.size() == write_descriptor_sets[write_index].descriptorCount, "Invalid buffer_info size: {0}, should be {1}", buffer_info.size(), write_descriptor_sets[write_index].descriptorCount);
	this->buffer_infos[write_index] = buffer_info;
	write_descriptor_sets[write_index].pBufferInfo = this->buffer_infos[write_index].data();
}

DescriptorSet::DescriptorSet(const DescriptorSet& other)
{
	*this = other;
}

DescriptorSet& DescriptorSet::operator=(const DescriptorSet& other)
{
	layout = other.layout;
	VkDescriptorSetAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorPool = *Graphics::descriptor_pool;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &layout->layout;

	Graphics::vulkanAssert(vkAllocateDescriptorSets(*Graphics::logical_device, &allocation_info, &descriptor_set));

	image_infos = other.image_infos;
	buffer_infos = other.buffer_infos;
	write_descriptor_sets = other.write_descriptor_sets;
	for (size_t i = 0; i < write_descriptor_sets.size(); ++i)
	{
		write_descriptor_sets[i].dstSet = descriptor_set;
		write_descriptor_sets[i].pImageInfo = image_infos[i].data();
		write_descriptor_sets[i].pBufferInfo = buffer_infos[i].data();

	}

	descriptor_layout_bindings = other.descriptor_layout_bindings;

	return *this;
}