#include "descriptor_set.h"
#include "descriptor_allocator.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"
#include "gfx/descriptors/descriptor_pool.h"
#include "gfx/buffers/command_buffer.h"

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout) 
	: layout(layout), pool(DescriptorAllocator::allocate(descriptor_set, layout))
{
	
}

DescriptorSet::~DescriptorSet()
{
	pool->deallocate();
}

void DescriptorSet::update(const std::vector<Write>& writes) const
{
	VkWriteDescriptorSet default_write;
	default_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	default_write.pNext = nullptr;
	default_write.dstSet = descriptor_set;
	default_write.pImageInfo = nullptr;
	default_write.pBufferInfo = nullptr;
	default_write.pTexelBufferView = nullptr;
	default_write.dstArrayElement = 0; // TODO: support array indices
	std::vector<VkWriteDescriptorSet> descriptor_set_writes(writes.size(), default_write);
	for (size_t i = 0; i < writes.size(); ++i)
	{
		const auto& binding = layout.getBindings().at(writes[i].binding);
		auto& write = descriptor_set_writes[i]; 
		write.dstBinding = writes[i].binding;
		write.descriptorCount = binding.descriptorCount; // TODO: This should not always be set to this I think, should be fine for now
		write.descriptorType = binding.descriptorType;
		switch (write.descriptorType)
		{
		case VK_DESCRIPTOR_TYPE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			write.pImageInfo = (const VkDescriptorImageInfo*)writes[i].info;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
			write.pBufferInfo = (const VkDescriptorBufferInfo*)writes[i].info;
			break;
		case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: // TODO: Properly support texel buffers
		case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			write.pTexelBufferView = (const VkBufferView*)writes[i].info;
			break;
		}
	}
	RenderContext::getLogicalDevice().updateDescriptorSets(descriptor_set_writes);
}

void DescriptorSet::bind(size_t first_set)
{
	RenderContext::submit([&](CommandBuffer& cb) { cb.bindDescriptorSets(first_set, { descriptor_set }); });
}