#include "descriptor_set.h"
#include "graphics.h"
#include "graphics_state.h"

DescriptorSet::DescriptorSet(VkDescriptorSetLayout layout, size_t count)
{
	descriptor_sets.resize(count);
	std::vector<VkDescriptorSetLayout> layouts(count, layout);
	VkDescriptorSetAllocateInfo allocation_info{};
	allocation_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocation_info.descriptorPool = *Graphics::descriptor_pool;
	allocation_info.descriptorSetCount = count;
	allocation_info.pSetLayouts = layouts.data();

	Graphics::vulkanAssert(vkAllocateDescriptorSets(*Graphics::logical_device, &allocation_info, descriptor_sets.data()));

	for (size_t i = 0; i < count; i++) //SUPER HARDCODED
	{
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = *Graphics::uniform_buffers[i];
		buffer_info.offset = 0;
		buffer_info.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = descriptor_sets[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &buffer_info;

		vkUpdateDescriptorSets(*Graphics::logical_device, 1, &descriptor_write, 0, nullptr);
	}
}

void DescriptorSet::bind(size_t index)
{
	vkCmdBindDescriptorSets(*graphics_state.command_buffer, *graphics_state.bind_point, Graphics::graphics_pipeline->getLayout(), 0, 1, &descriptor_sets[index], 0, nullptr);
}
