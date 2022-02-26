#pragma once

#include "descriptor_set_layout.h"
#include "gfx/images/image.h"
#include "gfx/images/image_array.h"

class DescriptorSet : NonMovable
{
public:
	DescriptorSet() = default;
	DescriptorSet(const DescriptorSet & other);
	DescriptorSet& operator=(const DescriptorSet & other);

	DescriptorSet& add(uint32_t binding, uint32_t count, vk::DescriptorType descriptor_type, vk::ShaderStageFlags stage_flags);
	void build();

	void bind(size_t first_set = 0);

	bool wasUpdated() const { return updated; }
	const vk::DescriptorSetLayout& getLayout() const { return *layout; }
	std::vector<vk::WriteDescriptorSet>& getWrites() { return write_descriptor_sets; }
	operator const vk::DescriptorSet& () const { return descriptor_set; }

	void setImageInfo(size_t write_index, const std::vector<vk::DescriptorImageInfo>& image_info);
	void setBufferInfo(size_t write_index, const std::vector<vk::DescriptorBufferInfo>& buffer_info);

private:
	void update();
	void forceUpdate();

private:
	vk::DescriptorSet descriptor_set;
	shared<DescriptorSetLayout> layout;
	std::vector<vk::WriteDescriptorSet> write_descriptor_sets;
	std::vector<std::vector<vk::DescriptorImageInfo>> image_infos;
	std::vector<std::vector<vk::DescriptorBufferInfo>> buffer_infos;
	std::vector<vk::DescriptorSetLayoutBinding> descriptor_set_layout_bindings;
	bool updated = false;
	bool needs_update = true;
};