#pragma once

class DescriptorPool : NonCopyable
{
public:
	~DescriptorPool();

	DescriptorPool& addSize(VkDescriptorType type, uint32_t count);
	DescriptorPool& setMaxSets(uint32_t count);
	void build();

	VkResult allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout);
	void deallocate();

	operator const VkDescriptorPool& () const { return descriptor_pool; }
	uint32_t allocatedDescriptorSetCount() const { return allocated_descriptor_sets; }

private:
	VkDescriptorPool descriptor_pool = nullptr;
	std::vector<VkDescriptorPoolSize> sizes;
	uint32_t max_sets = 0;
	uint32_t allocated_descriptor_sets = 0;
};