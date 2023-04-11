#pragma once

#include "descriptor_set_layout.h"
#include "descriptor_pool.h"

class DescriptorSet : NonCopyable
{
public:
	struct Write
	{
		uint32_t binding = 0;
		const void* info = nullptr; // Either VkDescriptorBufferInfo or VkDescriptorImageInfo or VkBufferView
	};

public:
	DescriptorSet(const DescriptorSetLayout& layout);
	~DescriptorSet();

	void update(const std::vector<Write>& writes) const;

	void bind(size_t first_set = 0);

	const VkDescriptorSetLayout& getLayout() const { return layout; }
	operator const VkDescriptorSet& () const { return descriptor_set; }

private:
	VkDescriptorSet descriptor_set = nullptr;
	const DescriptorSetLayout& layout;
	shared<DescriptorPool> pool = nullptr;
};