#include "descriptor_allocator.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

void DescriptorAllocator::reset()
{
	allocated_descriptor_sets = 0;
	descriptor_sizes.clear();
	for (const auto& p : used_pools)
	{
		if (!p->allocatedDescriptorSetCount())
		{
			RenderContext::getLogicalDevice().resetDescriptorPool(*p);
			free_pools.emplace_back(p);
		}
		else allocated_descriptor_sets += p->allocatedDescriptorSetCount();
	}
	used_pools.clear();
	current_pool = nullptr;
}

shared<DescriptorPool> DescriptorAllocator::allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout)
{
	if (!current_pool)
		current_pool = grabPool();

	VkResult alloc_result = current_pool->allocate(descriptor_set, descriptor_set_layout);
	if (alloc_result == VK_ERROR_FRAGMENTED_POOL || alloc_result == VK_ERROR_OUT_OF_POOL_MEMORY)
	{
		current_pool = grabPool();
		alloc_result = current_pool->allocate(descriptor_set, descriptor_set_layout);
		if (alloc_result == VK_SUCCESS)
			return current_pool;
	}
	return current_pool;
}

void DescriptorAllocator::destroy()
{
	descriptor_sizes.clear();
	free_pools.clear();
	used_pools.clear();
	allocated_descriptor_sets = 0;
	current_pool = nullptr;
}

void DescriptorAllocator::trackUpdate(VkDescriptorType descriptor_type, uint32_t descriptor_count)
{
	descriptor_sizes[descriptor_type] += descriptor_count;
}

shared<DescriptorPool> DescriptorAllocator::createPool()
{
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(descriptor_sizes.size());
	for (auto&& [type, size] : descriptor_sizes)
		sizes.emplace_back(type, size * 2 + 4);
	shared<DescriptorPool> pool = makeShared<DescriptorPool>(allocated_descriptor_sets * 2 + 4, sizes);
	used_pools.push_back(pool);
	return pool;
}

shared<DescriptorPool> DescriptorAllocator::grabPool()
{
	if (free_pools.size())
	{
		shared<DescriptorPool> pool = free_pools.back();
		free_pools.pop_back();
		used_pools.push_back(pool);
		return pool;
	}
	return createPool();
}
