#pragma once

class DescriptorPool : NonCopyable
{
public:
	~DescriptorPool();

	DescriptorPool& addSize(vk::DescriptorType type, uint32_t count);
	DescriptorPool& setMaxSets(uint32_t count);
	void build();

	operator const vk::DescriptorPool& () const { return descriptor_pool; }

private:
	vk::DescriptorPool descriptor_pool;
	std::vector<vk::DescriptorPoolSize> sizes;
	uint32_t max_sets = 0;
};