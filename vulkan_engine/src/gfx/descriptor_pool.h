#pragma once

class DescriptorPool
{
public:
	~DescriptorPool();

	DescriptorPool& addSize(VkDescriptorType type, uint32_t count);
	DescriptorPool& setMaxSets(uint32_t count);
	void build();

	operator const VkDescriptorPool& () { return descriptor_pool; }

private:
	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorPoolSize> sizes;
	uint32_t max_sets = 0;
};