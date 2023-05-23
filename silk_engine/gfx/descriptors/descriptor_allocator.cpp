#include "descriptor_allocator.h"
#include "silk_engine/gfx/render_context.h"
#include "silk_engine/gfx/devices/logical_device.h"

void DescriptorAllocator::reset()
{
	allocated_descriptor_sets = 0;
	descriptor_sizes.clear();
	for (const auto& p : used_pools)
	{
		allocated_descriptor_sets += p->allocatedDescriptorSetCount();
		if (!p->allocatedDescriptorSetCount())
		{
			p->reset();
			free_pools.emplace_back(p);
		}
	}
	used_pools.clear();
	current_pool = nullptr;
}

shared<DescriptorPool> DescriptorAllocator::allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout)
{
	if (!current_pool)
		current_pool = getPool();
	VkResult alloc_result = current_pool->allocate(descriptor_set, descriptor_set_layout);
	if (alloc_result == VK_SUCCESS)
		return current_pool;
	if (alloc_result == VK_ERROR_FRAGMENTED_POOL || alloc_result == VK_ERROR_OUT_OF_POOL_MEMORY)
	{
		current_pool = nullptr;
		return allocate(descriptor_set, descriptor_set_layout);
	}
	return nullptr;
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

shared<DescriptorPool> DescriptorAllocator::getPool()
{
	if (free_pools.size())
	{
		shared<DescriptorPool> pool = free_pools.back();
		free_pools.pop_back();
		used_pools.push_back(pool);
		return pool;
	}

	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(descriptor_sizes.size());
	for (auto&& [type, size] : descriptor_sizes)
		sizes.emplace_back(type, size);
	shared<DescriptorPool> pool = makeShared<DescriptorPool>(1024, sizes);
	used_pools.push_back(pool);
	return pool;
}
