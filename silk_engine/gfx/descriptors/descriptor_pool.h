#pragma once

class DescriptorPool : NoCopy
{
public:
	DescriptorPool(uint32_t max_sets, const std::vector<VkDescriptorPoolSize>& sizes);
	~DescriptorPool();

	VkResult allocate(VkDescriptorSet& descriptor_set, const VkDescriptorSetLayout& descriptor_set_layout) const;
	void deallocate() const;

	void reset() const;

	operator const VkDescriptorPool& () const { return descriptor_pool; }
	uint32_t allocatedDescriptorSetCount() const { return allocated_descriptor_sets; }

private:
	VkDescriptorPool descriptor_pool = nullptr;
	mutable uint32_t allocated_descriptor_sets = 0;
};