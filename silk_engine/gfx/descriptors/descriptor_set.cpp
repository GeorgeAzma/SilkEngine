#include "descriptor_set.h"
#include "descriptor_allocator.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"
#include "gfx/descriptors/descriptor_pool.h"
#include "scene/resources.h"
#include "gfx/buffers/command_buffer.h"

DescriptorSet::~DescriptorSet()
{
	pool->deallocate();
}

DescriptorSet& DescriptorSet::add(uint32_t binding, uint32_t count, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags)
{
	switch (descriptor_type)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: 
		buffer_infos.emplace_back();
		image_infos.emplace_back(count);
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: 
		buffer_infos.emplace_back(count);
		image_infos.emplace_back();
		break;
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: // TODO: Properly support texel buffers
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		buffer_infos.emplace_back();
		image_infos.emplace_back();
		break;
	}

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
	//descriptor_layout_binding.pImmutableSamplers = ; //TODO: Support immutable samplers
	descriptor_set_layout_bindings.emplace_back(std::move(descriptor_layout_binding));

	return *this;
}

void DescriptorSet::build()
{
	if (descriptor_set_layout_bindings.empty())
		return;

	layout = Resources::getDescriptorSetLayout(descriptor_set_layout_bindings);
	pool = DescriptorAllocator::allocate(descriptor_set, *layout);
	needs_update = true;

	for (auto& write : write_descriptor_sets)
		write.dstSet = descriptor_set;
}

void DescriptorSet::update()
{
	if (needs_update)
		forceUpdate();
}

void DescriptorSet::forceUpdate()
{
	Graphics::logical_device->updateDescriptorSets(write_descriptor_sets);
	needs_update = false;
}

void DescriptorSet::bind(size_t first_set)
{
	update();
	Graphics::submit([&](CommandBuffer& cb) { cb.bindDescriptorSets(first_set, { descriptor_set }); });
}

void DescriptorSet::setImageInfo(size_t binding, const std::vector<VkDescriptorImageInfo>& image_info)
{
	for (size_t i = 0; i < write_descriptor_sets.size(); ++i)
	{
		if (write_descriptor_sets[i].dstBinding == binding)
		{
			SK_VERIFY(image_info.size() == write_descriptor_sets[i].descriptorCount, "Invalid image_info size: {}, should be {}", image_info.size(), write_descriptor_sets[i].descriptorCount);
			std::copy(image_info.begin(), image_info.end(), image_infos[i].begin());
			write_descriptor_sets[i].pImageInfo = image_infos[i].data();
			needs_update = true;
			break;
		}
	}
}

void DescriptorSet::setBufferInfo(size_t binding, const std::vector<VkDescriptorBufferInfo>& buffer_info)
{
	for (size_t i = 0; i < write_descriptor_sets.size(); ++i)
	{
		if (write_descriptor_sets[i].dstBinding == binding)
		{
			SK_VERIFY(buffer_info.size() == write_descriptor_sets[i].descriptorCount, "Invalid buffer_info size: {}, should be {}", buffer_info.size(), write_descriptor_sets[i].descriptorCount);
			std::copy(buffer_info.begin(), buffer_info.end(), buffer_infos[i].begin());
			write_descriptor_sets[i].pBufferInfo = buffer_infos[i].data();
			needs_update = true;
			break;
		}
	}
}

DescriptorSet::DescriptorSet(const DescriptorSet& other)
{
	*this = other;
}

DescriptorSet& DescriptorSet::operator=(const DescriptorSet& other)
{
	layout = other.layout;
	pool = DescriptorAllocator::allocate(descriptor_set, layout->layout);

	image_infos = other.image_infos;
	buffer_infos = other.buffer_infos;
	write_descriptor_sets = other.write_descriptor_sets;
	for (size_t i = 0; i < write_descriptor_sets.size(); ++i)
	{
		write_descriptor_sets[i].dstSet = descriptor_set;
		write_descriptor_sets[i].pImageInfo = image_infos[i].data();
		write_descriptor_sets[i].pBufferInfo = buffer_infos[i].data();
	}
	needs_update = true;

	descriptor_set_layout_bindings = other.descriptor_set_layout_bindings;

	return *this;
}