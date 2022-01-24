#include "descriptor_set.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/descriptors/descriptor_pool.h"

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout)
	: layout{&layout}
{
	VkDescriptorSetAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorPool = *Graphics::descriptor_pool;
	allocation_info.descriptorSetCount = 1;
	allocation_info.pSetLayouts = &(const VkDescriptorSetLayout&)layout;

	Graphics::vulkanAssert(vkAllocateDescriptorSets(*Graphics::logical_device, &allocation_info, &descriptor_set)); //This can be done it descriptor pool
}

DescriptorSet& DescriptorSet::addBuffer(uint32_t binding, VkDescriptorBufferInfo buffer_info)
{
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = descriptor_set;
	descriptor_write.dstBinding = binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = layout->bindings.at(binding).descriptorType;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &buffer_info;
	descriptor_write.pImageInfo = nullptr;
	descriptor_writes.emplace_back(std::move(descriptor_write));

	return *this;
}

//TODO:
DescriptorSet& DescriptorSet::addImage(uint32_t binding, const VkDescriptorImageInfo& descriptor_image_info)
{
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = descriptor_set;
	descriptor_write.dstBinding = binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = layout->bindings.at(binding).descriptorType;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pImageInfo = &descriptor_image_info;
	descriptor_writes.emplace_back(std::move(descriptor_write));

	return *this;
}

DescriptorSet& DescriptorSet::addImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& descriptor_image_info)
{
	VkWriteDescriptorSet descriptor_write{};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = descriptor_set;
	descriptor_write.dstBinding = binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = layout->bindings.at(binding).descriptorType;
	descriptor_write.descriptorCount = descriptor_image_info.size();
	descriptor_write.pImageInfo = descriptor_image_info.data();
	descriptor_writes.emplace_back(std::move(descriptor_write));

	return *this;
}

void DescriptorSet::build()
{
	vkUpdateDescriptorSets(*Graphics::logical_device, descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
}

void DescriptorSet::bind()
{
	vkCmdBindDescriptorSets(Graphics::active.command_buffer, Graphics::active.bind_point, Graphics::active.pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
	
	Graphics::active.descriptor_set = descriptor_set;
}
