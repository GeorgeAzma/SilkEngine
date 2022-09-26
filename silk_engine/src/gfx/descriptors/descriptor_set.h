#pragma once

#include "descriptor_set_layout.h"
#include "descriptor_pool.h"

class DescriptorSet : NonMovable
{
public:
	DescriptorSet() = default;
	~DescriptorSet();
	DescriptorSet(const DescriptorSet & other);
	DescriptorSet& operator=(const DescriptorSet & other);

	DescriptorSet& add(uint32_t binding, uint32_t count, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags);
	void build();

	void bind(size_t first_set = 0);

	bool wasUpdated() const { return !needs_update; }
	const VkDescriptorSetLayout& getLayout() const { return *layout; }
	std::vector<VkWriteDescriptorSet>& getWrites() { return write_descriptor_sets; }
	operator const VkDescriptorSet& () const { return descriptor_set; }

	void setImageInfo(size_t binding, const std::vector<VkDescriptorImageInfo>& image_info);
	void setBufferInfo(size_t binding, const std::vector<VkDescriptorBufferInfo>& buffer_info);

private:
	void update();
	void forceUpdate();

private:
	VkDescriptorSet descriptor_set = nullptr;
	shared<DescriptorSetLayout> layout = nullptr;
	shared<DescriptorPool> pool = nullptr;
	std::vector<VkWriteDescriptorSet> write_descriptor_sets;
	std::vector<std::vector<VkDescriptorImageInfo>> image_infos;
	std::vector<std::vector<VkDescriptorBufferInfo>> buffer_infos;
	std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
	bool needs_update = true;
};