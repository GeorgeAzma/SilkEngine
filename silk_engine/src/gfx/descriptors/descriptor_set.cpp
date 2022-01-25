#include "descriptor_set.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/descriptors/descriptor_pool.h"
#include "scene/resources.h"

DescriptorSet& DescriptorSet::addBuffer(uint32_t binding, VkDescriptorBufferInfo buffer_info, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstBinding = binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = descriptor_type;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &buffer_info;
	descriptor_write.pImageInfo = nullptr;
	descriptor_writes.emplace_back(std::move(descriptor_write));

	VkDescriptorSetLayoutBinding descriptor_layout_binding{};
	descriptor_layout_binding.binding = binding;
	descriptor_layout_binding.descriptorCount = 1;
	descriptor_layout_binding.descriptorType = descriptor_type;
	descriptor_layout_binding.stageFlags = stage_flags;
	descriptor_layout_bindings.emplace_back(std::move(descriptor_layout_binding));

	return *this;
}

DescriptorSet& DescriptorSet::addImage(uint32_t binding, const VkDescriptorImageInfo& descriptor_image_info, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstBinding = binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = descriptor_type;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pImageInfo = &descriptor_image_info;
	descriptor_writes.emplace_back(std::move(descriptor_write));

	VkDescriptorSetLayoutBinding descriptor_layout_binding{};
	descriptor_layout_binding.binding = binding;
	descriptor_layout_binding.descriptorCount = 1;
	descriptor_layout_binding.descriptorType = descriptor_type;
	descriptor_layout_binding.stageFlags = stage_flags;
	descriptor_layout_bindings.emplace_back(std::move(descriptor_layout_binding));

	return *this;
}

DescriptorSet& DescriptorSet::addImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& descriptor_image_info, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstBinding = binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = descriptor_type;
	descriptor_write.descriptorCount = descriptor_image_info.size();
	descriptor_write.pImageInfo = descriptor_image_info.data();
	descriptor_writes.emplace_back(std::move(descriptor_write));

	VkDescriptorSetLayoutBinding descriptor_layout_binding{};
	descriptor_layout_binding.binding = binding;
	descriptor_layout_binding.descriptorCount = descriptor_image_info.size();
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

	for (auto& write : descriptor_writes)
		write.dstSet = descriptor_set;

	vkUpdateDescriptorSets(*Graphics::logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
}

void DescriptorSet::bind(size_t first_set)
{
	vkCmdBindDescriptorSets(Graphics::active.command_buffer, Graphics::active.bind_point, Graphics::active.pipeline_layout, first_set, 1, &descriptor_set, 0, nullptr);
}