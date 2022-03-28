#pragma once

#include "descriptor_pool.h"

class DescriptorAllocator
{
	struct PoolSizes
	{
		std::vector<std::pair<vk::DescriptorType, float>> sizes =
		{
			{ vk::DescriptorType::eSampler, 1.0f },
			{ vk::DescriptorType::eCombinedImageSampler, 4.0f },
			{ vk::DescriptorType::eSampledImage, 1.0f },
			{ vk::DescriptorType::eStorageImage, 4.0f },
			{ vk::DescriptorType::eUniformTexelBuffer, 1.0f },
			{ vk::DescriptorType::eStorageTexelBuffer, 1.0f },
			{ vk::DescriptorType::eUniformBuffer, 4.0f },
			{ vk::DescriptorType::eStorageBuffer, 4.0f },
			{ vk::DescriptorType::eUniformBufferDynamic, 1.0f },
			{ vk::DescriptorType::eStorageBufferDynamic, 1.0f },
			{ vk::DescriptorType::eInputAttachment, 1.0f }
		};
	};
public:
	static void reset();
	static shared<DescriptorPool> allocate(vk::DescriptorSet& descriptor_set, const vk::DescriptorSetLayout& descriptor_set_layout);
	static void cleanup();

private:
	static shared<DescriptorPool> createPool();
	static shared<DescriptorPool> grabPool();

private:
	static inline shared<DescriptorPool> current_pool = nullptr;
	static inline PoolSizes descriptor_sizes = {};
	static inline std::vector<shared<DescriptorPool>> used_pools;
	static inline std::vector<shared<DescriptorPool>> free_pools;
};