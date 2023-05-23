#include "descriptor_set.h"
#include "descriptor_allocator.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"
#include "silk_engine/gfx/descriptors/descriptor_pool.h"
#include "silk_engine/gfx/buffers/command_buffer.h"

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout) 
	: layout(layout), pool(DescriptorAllocator::allocate(descriptor_set, layout))
{
	for (auto&& [binding, binding_info] : layout.getBindings())
	{
		buffer_infos[binding].resize(binding_info.descriptorCount, VkDescriptorBufferInfo{ .buffer = nullptr, .offset = 0, .range = 0 });
		image_infos[binding].resize(binding_info.descriptorCount, VkDescriptorImageInfo{ .sampler = nullptr, .imageView = nullptr, .imageLayout = VK_IMAGE_LAYOUT_MAX_ENUM });
		buffer_views[binding].resize(binding_info.descriptorCount, nullptr);
	}
}

DescriptorSet::~DescriptorSet()
{
	pool->deallocate();
}

void DescriptorSet::write(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& buffer_infos, uint32_t array_index)
{
	for (size_t i = 0; i < buffer_infos.size(); ++i)
	{
		VkDescriptorBufferInfo& existing_buffer = this->buffer_infos.at(binding)[array_index + i];
		if (existing_buffer.buffer != buffer_infos[i].buffer || 
			existing_buffer.range  != buffer_infos[i].range  || 
			existing_buffer.offset != buffer_infos[i].offset)
		{
			existing_buffer = buffer_infos[i];
			write(binding, array_index + i, 1, &existing_buffer);
		}
	}
}

void DescriptorSet::write(uint32_t binding, const std::vector<VkDescriptorImageInfo>& image_infos, uint32_t array_index)
{
	for (size_t i = 0; i < image_infos.size(); ++i)
	{
		VkDescriptorImageInfo& existing_image = this->image_infos.at(binding)[array_index + i];
		if (existing_image.imageLayout != image_infos[i].imageLayout ||
			existing_image.imageView   != image_infos[i].imageView ||
			existing_image.sampler     != image_infos[i].sampler)
		{
			existing_image = image_infos[i];
			write(binding, array_index + i, 1, nullptr, &existing_image);
		}
	}
}

void DescriptorSet::write(uint32_t binding, const std::vector<VkBufferView>& buffer_views, uint32_t array_index)
{
	for (size_t i = 0; i < image_infos.size(); ++i)
	{
		VkBufferView& existing_buffer_view = this->buffer_views.at(binding)[array_index + i];
		if (existing_buffer_view != buffer_views[i])
		{
			existing_buffer_view = buffer_views[i];
			write(binding, array_index + i, 1, nullptr, nullptr, &existing_buffer_view);
		}
	}
}

void DescriptorSet::write(uint32_t binding, const VkDescriptorBufferInfo& buffer_info, uint32_t array_index)
{
	write(binding, std::vector<VkDescriptorBufferInfo> { buffer_info }, array_index);
}

void DescriptorSet::write(uint32_t binding, const VkDescriptorImageInfo& image_info, uint32_t array_index)
{
	write(binding, std::vector<VkDescriptorImageInfo> { image_info }, array_index);
}

void DescriptorSet::write(uint32_t binding, const VkBufferView& buffer_view, uint32_t array_index)
{
	write(binding, std::vector<VkBufferView> { buffer_view }, array_index);
}

void DescriptorSet::update()
{
	if (writes.empty())
		return;
	RenderContext::getLogicalDevice().updateDescriptorSets(writes);
	for (const auto& write : writes)
		DescriptorAllocator::trackUpdate(write.descriptorType, write.descriptorCount);
	writes.clear();
}

void DescriptorSet::bind(size_t first, const std::vector<uint32_t> dynamic_offsets)
{
	SK_ASSERT(dynamic_offsets.size() == layout.getDynamicDescriptorCount());
	update();
	RenderContext::getCommandBuffer().bindDescriptorSets(first, { descriptor_set }, dynamic_offsets);
}

void DescriptorSet::write(uint32_t binding, uint32_t array_index, uint32_t descriptor_count, const VkDescriptorBufferInfo* buffer_info, const VkDescriptorImageInfo* image_info, const VkBufferView* buffer_view)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_index;
	write.pBufferInfo = buffer_info;
	write.pImageInfo = image_info;
	write.pTexelBufferView = buffer_view;

	const auto& layout_binding = layout.getBindings().at(binding);
	write.descriptorCount = descriptor_count ? descriptor_count : layout_binding.descriptorCount;
	write.descriptorType = layout_binding.descriptorType; 

	writes.emplace_back(std::move(write));
}
