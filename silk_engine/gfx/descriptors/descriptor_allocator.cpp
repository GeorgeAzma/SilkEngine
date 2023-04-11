#include "descriptor_allocator.h"
#include "gfx/render_context.h"
#include "gfx/devices/logical_device.h"

void DescriptorAllocator::reset()
{
	for (const auto& p : used_pools)
	{
		if (!p->allocatedDescriptorSetCount())
		{
			RenderContext::getLogicalDevice().resetDescriptorPool(*p);
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
	current_pool = nullptr;
	free_pools.clear();
	used_pools.clear();
}

shared<DescriptorPool> DescriptorAllocator::createPool()
{
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(descriptor_sizes.sizes.size());
	for (auto&& [type, size] : descriptor_sizes.sizes)
		sizes.emplace_back(type, uint32_t(size * 32.0f));
	shared<DescriptorPool> pool = makeShared<DescriptorPool>(64, sizes);
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
