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
		bool operator==(const Write& other) const { return info == other.info && binding == other.binding; }
	};

public:
	DescriptorSet(const DescriptorSetLayout& layout);
	~DescriptorSet();

	void write(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& buffer_infos, size_t array_index = 0);
	void write(uint32_t binding, const std::vector<VkDescriptorImageInfo>& image_infos, size_t array_index = 0);
	void write(uint32_t binding, const std::vector<VkBufferView>& buffer_views, size_t array_index = 0);
	void write(uint32_t binding, const VkDescriptorBufferInfo& buffer_info, size_t array_index = 0);
	void write(uint32_t binding, const VkDescriptorImageInfo& image_info, size_t array_index = 0);
	void write(uint32_t binding, const VkBufferView& buffer_view, size_t array_index = 0);
	void update();

	void bind(size_t first_set = 0);

	const VkDescriptorSetLayout& getLayout() const { return layout; }
	operator const VkDescriptorSet& () const { return descriptor_set; }

private:
	void write(uint32_t binding, uint32_t array_index = 0, uint32_t descriptor_count = 0);

private:
	VkDescriptorSet descriptor_set = nullptr;
	const DescriptorSetLayout& layout;
	shared<DescriptorPool> pool = nullptr;
	std::vector<VkWriteDescriptorSet> writes{};
	std::unordered_map<uint32_t, std::vector<VkDescriptorBufferInfo>> old_buffer_infos;
	std::unordered_map<uint32_t, std::vector<VkDescriptorImageInfo>> old_image_infos;
	std::unordered_map<uint32_t, std::vector<VkBufferView>> old_buffer_views;
	std::unordered_map<uint32_t, std::vector<VkDescriptorBufferInfo>> buffer_infos;
	std::unordered_map<uint32_t, std::vector<VkDescriptorImageInfo>> image_infos;
	std::unordered_map<uint32_t, std::vector<VkBufferView>> buffer_views;
};