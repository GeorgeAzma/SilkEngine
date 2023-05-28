#pragma once

#include "descriptor_set_layout.h"
#include "descriptor_pool.h"

class DescriptorSet : NoCopy
{
public:
	DescriptorSet(const DescriptorSetLayout& layout);
	~DescriptorSet();

	void write(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& buffer_infos, uint32_t array_index = 0);
	void write(uint32_t binding, const std::vector<VkDescriptorImageInfo>& image_infos, uint32_t array_index = 0);
	void write(uint32_t binding, const std::vector<VkBufferView>& buffer_views, uint32_t array_index = 0);
	void write(uint32_t binding, const VkDescriptorBufferInfo& buffer_info, uint32_t array_index = 0);
	void write(uint32_t binding, const VkDescriptorImageInfo& image_info, uint32_t array_index = 0);
	void write(uint32_t binding, const VkBufferView& buffer_view, uint32_t array_index = 0); 
	void update();
	
	void bind(size_t first = 0, const std::vector<uint32_t>& dynamic_offsets = {});

	const VkDescriptorSetLayout& getLayout() const { return layout; }
	operator const VkDescriptorSet& () const { return descriptor_set; }

private:
	void write(uint32_t binding, uint32_t array_index = 0, uint32_t descriptor_count = 0, const VkDescriptorBufferInfo* buffer_info = nullptr, const VkDescriptorImageInfo* image_info = nullptr, const VkBufferView* buffer_view = nullptr);

private:
	VkDescriptorSet descriptor_set = nullptr;
	const DescriptorSetLayout& layout;
	shared<DescriptorPool> pool = nullptr;
	std::vector<VkWriteDescriptorSet> writes{};
	std::unordered_map<uint32_t, std::vector<VkDescriptorBufferInfo>> buffer_infos;
	std::unordered_map<uint32_t, std::vector<VkDescriptorImageInfo>> image_infos;
	std::unordered_map<uint32_t, std::vector<VkBufferView>> buffer_views;
};