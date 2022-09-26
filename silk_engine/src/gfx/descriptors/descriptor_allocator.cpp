#include "descriptor_allocator.h"
#include "gfx/graphics.h"
#include "gfx/devices/logical_device.h"

void DescriptorAllocator::reset()
{
	for (const auto& p : used_pools)
	{
		if (!p->allocatedDescriptorSetCount())
		{
			Graphics::logical_device->resetDescriptorPool(*p);
			free_pools.emplace_back(p);
		}
	}
	used_pools.clear();
	current_pool = nullptr;
}

shared<DescriptorPool> DescriptorAllocator::allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout)
{
	if (!current_pool)
		current_pool = grabPool();

	VkResult alloc_result = current_pool->allocate(descriptor_set, descriptor_set_layout);
	bool need_realloc = false;
	switch (alloc_result)
	{
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			need_realloc = true;
			break;
		default:
			return current_pool;
	}

	if (need_realloc)
	{
		current_pool = grabPool();
		alloc_result = current_pool->allocate(descriptor_set, descriptor_set_layout);
		if (alloc_result == VK_SUCCESS)
			return current_pool;
	}
	
	return nullptr;
}

void DescriptorAllocator::destroy()
{
	current_pool = nullptr;
	free_pools.clear();
	used_pools.clear();
}

shared<DescriptorPool> DescriptorAllocator::createPool()
{
	shared<DescriptorPool> pool = makeShared<DescriptorPool>();
	for(const auto& size : descriptor_sizes.sizes)
		pool->addSize(size.first, size.second * 64);
	pool->setMaxSets(256);
	pool->build();
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
