#include "descriptor_set.h"
#include "graphics.h"
#include "graphics_state.h"

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout, size_t count)
	: layout{&layout}
{
	descriptor_sets.resize(count);
	std::vector<VkDescriptorSetLayout> layouts(count, (const VkDescriptorSetLayout&)layout);
	VkDescriptorSetAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorPool = *Graphics::descriptor_pool;
	allocation_info.descriptorSetCount = count;
	allocation_info.pSetLayouts = layouts.data();

	Graphics::vulkanAssert(vkAllocateDescriptorSets(*Graphics::logical_device, &allocation_info, descriptor_sets.data())); //This can be done it descriptor pool
}

DescriptorSet& DescriptorSet::addBuffer(uint32_t binding, VkDescriptorBufferInfo buffer_info)
{
	for (size_t i = 0; i < descriptor_sets.size(); ++i)
	{
		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = descriptor_sets[i];
		descriptor_write.dstBinding = binding;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = layout->bindings.at(binding).descriptorType;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &buffer_info;
		descriptor_write.pImageInfo = nullptr;
		descriptor_writes.emplace_back(std::move(descriptor_write));
	}

	return *this;
}

DescriptorSet& DescriptorSet::addImage(uint32_t binding, VkDescriptorImageInfo image_info)
{
	for (size_t i = 0; i < descriptor_sets.size(); ++i)
	{
		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = descriptor_sets[i];
		descriptor_write.dstBinding = binding;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = layout->bindings.at(binding).descriptorType;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pImageInfo = &image_info;
		descriptor_writes.emplace_back(std::move(descriptor_write));
	}

	return *this;
}

void DescriptorSet::build()
{
	vkUpdateDescriptorSets(*Graphics::logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
}

void DescriptorSet::bind(size_t index)
{
	vkCmdBindDescriptorSets(*graphics_state.command_buffer, *graphics_state.bind_point, Graphics::graphics_pipeline->getLayout(), 0, 1, &descriptor_sets[index], 0, nullptr);
}
