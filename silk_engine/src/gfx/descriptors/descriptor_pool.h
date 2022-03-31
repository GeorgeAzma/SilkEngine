#pragma once

class DescriptorPool : NonCopyable
{
public:
	~DescriptorPool();

	DescriptorPool& addSize(vk::DescriptorType type, uint32_t count);
	DescriptorPool& setMaxSets(uint32_t count);
	void build();

	vk::Result allocate(vk::DescriptorSet& descriptor_set, const vk::DescriptorSetLayout& descriptor_set_layout);
	void deallocate();

	operator const vk::DescriptorPool& () const { return descriptor_pool; }
	uint32_t allocatedDescriptorSetCount() const { return allocated_descriptor_sets; }

private:
	vk::DescriptorPool descriptor_pool;
	std::vector<vk::DescriptorPoolSize> sizes;
	uint32_t max_sets = 0;
	uint32_t allocated_descriptor_sets = 0;
};