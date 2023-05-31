#pragma once

#include "descriptor_pool.h"

class DescriptorAllocator
{
public:
	static void reset();
	static shared<DescriptorPool> allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout);
	static void destroy();
	static void trackUpdate(VkDescriptorType descriptor_type, uint32_t descriptor_count);

private:
	static shared<DescriptorPool> getPool();

private:
	static inline shared<DescriptorPool> current_pool = nullptr;
	static inline std::unordered_map<VkDescriptorType, uint32_t> descriptor_sizes =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 8 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 16 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 8 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 8 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 32 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 64 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 8 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 8 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 32 }
	};
	static inline std::vector<shared<DescriptorPool>> used_pools;
	static inline std::vector<shared<DescriptorPool>> free_pools;
	static inline uint32_t allocated_descriptor_sets = 0;
};