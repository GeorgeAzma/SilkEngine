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

void DescriptorSet::write(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& buffer_infos, size_t array_index)
{
	auto it = old_buffer_infos.emplace(binding, buffer_infos);
	if (!it.second)
	{
		size_t i;
		for (i = 0; i < it.first->second.size(); ++i)
		{
			const auto& dbi = it.first->second[i];
			if (dbi.buffer != buffer_infos[i].buffer ||
				dbi.offset != buffer_infos[i].offset ||
				dbi.range != buffer_infos[i].range)
				break;
		}
		if (i >= it.first->second.size())
			return;
	}
	write(binding, array_index, buffer_infos.size());
	this->buffer_infos.emplace(binding, buffer_infos);
}

void DescriptorSet::write(uint32_t binding, const std::vector<VkDescriptorImageInfo>& image_infos, size_t array_index)
{
	auto it = old_image_infos.emplace(binding, image_infos);
	if (!it.second)
	{
		size_t i;
		for (i = 0; i < it.first->second.size(); ++i)
		{
			const auto& dii = it.first->second[i];
			if (dii.imageLayout != image_infos[i].imageLayout ||
				dii.imageView != image_infos[i].imageView ||
				dii.sampler != image_infos[i].sampler)
				break;
		}
		if (i >= it.first->second.size())
			return;
	}
	write(binding, array_index, image_infos.size());
	this->image_infos.emplace(binding, image_infos);
}

void DescriptorSet::write(uint32_t binding, const std::vector<VkBufferView>& buffer_views, size_t array_index)
{
	auto it = old_buffer_views.emplace(binding, buffer_views);
	if (!it.second)
	{
		size_t i;
		for (i = 0; i < it.first->second.size(); ++i)
			if (it.first->second[i] != buffer_views[i])
				break;
		if (i >= it.first->second.size())
			return;
	}
	write(binding, array_index, buffer_views.size());
	this->buffer_views.emplace(binding, buffer_views);
}

void DescriptorSet::write(uint32_t binding, const VkDescriptorBufferInfo& buffer_info, size_t array_index)
{
	write(binding, std::vector<VkDescriptorBufferInfo> { buffer_info }, array_index);
}

void DescriptorSet::write(uint32_t binding, const VkDescriptorImageInfo& image_info, size_t array_index)
{
	write(binding, std::vector<VkDescriptorImageInfo> { image_info }, array_index);
}

void DescriptorSet::write(uint32_t binding, const VkBufferView& buffer_view, size_t array_index)
{
	write(binding, std::vector<VkBufferView> { buffer_view }, array_index);
}

void DescriptorSet::update()
{
	if (writes.empty())
		return;

	for (auto& write : writes)
	{
		DescriptorAllocator::trackUpdate(write.descriptorType, write.descriptorCount);
		if (auto it = buffer_infos.find(write.dstBinding); it != buffer_infos.end())
			write.pBufferInfo = it->second.data();
		if (auto it = image_infos.find(write.dstBinding); it != image_infos.end())
			write.pImageInfo = it->second.data();
		if (auto it = buffer_views.find(write.dstBinding); it != buffer_views.end())
			write.pTexelBufferView = it->second.data();
	}

	RenderContext::getLogicalDevice().updateDescriptorSets(writes);

	old_buffer_infos = std::move(buffer_infos);
	buffer_infos = {};

	old_image_infos = std::move(image_infos);
	image_infos = {};

	old_buffer_views = std::move(buffer_views);
	buffer_views = {};

	writes.clear();
}

void DescriptorSet::bind(size_t first_set)
{
	update();
	RenderContext::record([&](CommandBuffer& cb) { cb.bindDescriptorSets(first_set, { descriptor_set }); });
}

void DescriptorSet::write(uint32_t binding, uint32_t array_index, uint32_t descriptor_count)
{
	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = descriptor_set;
	write.dstBinding = binding;
	write.dstArrayElement = array_index;

	const auto& layout_binding = layout.getBindings().at(binding);
	write.descriptorCount = descriptor_count ? descriptor_count : layout_binding.descriptorCount;
	write.descriptorType = layout_binding.descriptorType;

	writes.emplace_back(std::move(write));
}
