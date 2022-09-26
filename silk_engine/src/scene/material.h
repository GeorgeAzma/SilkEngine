#pragma once

#include "gfx/pipeline/graphics_pipeline.h"
#include "gfx/descriptors/descriptor_set.h"

struct Material : NonCopyable
{
	shared<GraphicsPipeline> pipeline;
	std::unordered_map<uint32_t, DescriptorSet> descriptor_sets;

	void set(std::string_view name, const VkDescriptorBufferInfo& buffer_info);
	void set(std::string_view name, const VkDescriptorImageInfo& image_info);

	void bind();

	//bool operator==(const Material& other) const { return pipeline == other.pipeline; }
};