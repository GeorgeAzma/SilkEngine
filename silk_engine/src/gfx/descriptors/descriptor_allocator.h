#pragma once

#include "descriptor_pool.h"

class DescriptorAllocator
{
	struct PoolSizes
	{
		std::vector<std::pair<VkDescriptorType, float>> sizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1.0f },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.0f },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1.0f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4.0f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.0f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.0f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4.0f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4.0f },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.0f },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.0f },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1.0f }
		};
	};
public:
	static void reset();
	static shared<DescriptorPool> allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout);
	static void destroy();

private:
	static shared<DescriptorPool> createPool();
	static shared<DescriptorPool> grabPool();

private:
	static inline shared<DescriptorPool> current_pool = nullptr;
	static inline PoolSizes descriptor_sizes = {};
	static inline std::vector<shared<DescriptorPool>> used_pools;
	static inline std::vector<shared<DescriptorPool>> free_pools;
};